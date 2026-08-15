#include "SceneEditor.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <locale>
#include <codecvt>
#include <iostream>

// Simple JSON writer + lightweight parser sufficient for round-tripping the saved format.
// Format produced:
// {
//   "version": 2,
//   "objects": [
//     {"type":"Capsule","position":[x,y,z],"rotation":[x,y,z],"scale":[x,y,z],"components":[{"type":"Physics","props":{"mass":"1.0"}}, {"type":"BoxCollider","props":{"halfExtents":[0.5,0.5,0.5]}}]},
//     ...
//   ]
// }

static std::string wstringToUtf8(const std::wstring& ws)
{
    // std::wstring_convert is deprecated in C++17 but available in C++14 projects
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
    return conv.to_bytes(ws);
}

bool SceneEditor::saveLevel(const std::wstring& filename, const std::vector<SerializedObject>& objects)
{
    const std::string path = wstringToUtf8(filename);
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) return false;
    // include a version number for forward/backward compatibility
    ofs << "{\n  \"version\": 2,\n  \"objects\": [\n";
    for (size_t i = 0; i < objects.size(); ++i) {
        const auto& o = objects[i];
        ofs << "    {\"type\":\"" << o.type << "\",";
        ofs << "\"position\":[" << std::setprecision(9) << o.position.m_x << "," << o.position.m_y << "," << o.position.m_z << "],";
        ofs << "\"rotation\":[" << o.rotation.m_x << "," << o.rotation.m_y << "," << o.rotation.m_z << "],";
        ofs << "\"scale\":[" << o.scale.m_x << "," << o.scale.m_y << "," << o.scale.m_z << "]";

        // components (optional)
        if (!o.components.empty()) {
            ofs << ",\"components\":[";
            for (size_t ci = 0; ci < o.components.size(); ++ci) {
                const auto& cr = o.components[ci];
                ofs << "{\"type\":\"" << cr.type << "\"";
                if (!cr.properties.empty()) {
                    ofs << ",\"props\":{";
                    for (size_t pi = 0; pi < cr.properties.size(); ++pi) {
                        const auto& kv = cr.properties[pi];
                        // write property value as raw JSON token if it looks like an array or number,
                        // otherwise quote it as string. To keep things simple here we assume caller
                        // already provided values in an appropriate raw format or as strings.
                        const std::string& key = kv.first;
                        const std::string& val = kv.second;
                        ofs << "\"" << key << "\":";
                        // Heuristic: if val starts with '[' or digit or '-' then write raw, else quote.
                        if (!val.empty() && (val[0] == '[' || val[0] == '-' || (val[0] >= '0' && val[0] <= '9'))) {
                            ofs << val;
                        } else {
                            ofs << "\"" << val << "\"";
                        }
                        if (pi + 1 < cr.properties.size()) ofs << ",";
                    }
                    ofs << "}";
                }
                ofs << "}";
                if (ci + 1 < o.components.size()) ofs << ",";
            }
            ofs << "]";
        }

        ofs << "}";
        if (i + 1 < objects.size()) ofs << ",\n";
        else ofs << "\n";
    }
    ofs << "  ]\n}\n";
    return ofs.good();
}

// Helpers for parsing
static void skipWhitespace(const std::string& s, size_t& pos)
{
    while (pos < s.size() && isspace((unsigned char)s[pos])) ++pos;
}

static bool parseArray3(const std::string& s, size_t& pos, Vector3D& out);

static bool parseString(const std::string& s, size_t& pos, std::string& out)
{
    skipWhitespace(s, pos);
    if (pos >= s.size() || s[pos] != '"') return false;
    ++pos;
    std::ostringstream oss;
    while (pos < s.size()) {
        char c = s[pos++];
        if (c == '"') { out = oss.str(); return true; }
        if (c == '\\' && pos < s.size()) {
            char esc = s[pos++];
            // handle simple escapes
            if (esc == 'n') oss << '\n';
            else if (esc == 't') oss << '\t';
            else if (esc == 'r') oss << '\r';
            else oss << esc;
        } else {
            oss << c;
        }
    }
    return false;
}

static bool parseArray3(const std::string& s, size_t& pos, Vector3D& out)
{
    skipWhitespace(s, pos);
    if (pos >= s.size() || s[pos] != '[') return false;
    ++pos;
    skipWhitespace(s, pos);
    // read three numbers separated by commas
    double vals[3] = {0.0, 0.0, 0.0};
    for (int i = 0; i < 3; ++i) {
        skipWhitespace(s, pos);
        // parse number
        size_t start = pos;
        // allow leading + or -
        if (pos < s.size() && (s[pos] == '+' || s[pos] == '-')) ++pos;
        bool seenDigit = false;
        while (pos < s.size() && (isdigit((unsigned char)s[pos]) || s[pos] == '.')) { ++pos; seenDigit = true; }
        // scientific notation
        if (pos < s.size() && (s[pos] == 'e' || s[pos] == 'E')) {
            ++pos;
            if (pos < s.size() && (s[pos] == '+' || s[pos] == '-')) ++pos;
            while (pos < s.size() && isdigit((unsigned char)s[pos])) ++pos;
        }
        if (!seenDigit) return false;
        std::string numstr = s.substr(start, pos - start);
        try { vals[i] = std::stod(numstr); }
        catch (...) { return false; }
        skipWhitespace(s, pos);
        if (i < 2) {
            if (pos >= s.size() || s[pos] != ',') return false;
            ++pos;
        }
    }
    skipWhitespace(s, pos);
    if (pos >= s.size() || s[pos] != ']') return false;
    ++pos;
    out.m_x = static_cast<float>(vals[0]);
    out.m_y = static_cast<float>(vals[1]);
    out.m_z = static_cast<float>(vals[2]);
    return true;
}

// Parse a props object like {"key":"value","num":1, "vec":[x,y,z]} into vector<pair<k,vstring>>.
// vstring will contain either a quoted string (without quotes), a numeric literal, or an array literal string.
static bool parsePropsObject(const std::string& s, size_t& pos, std::vector<std::pair<std::string, std::string>>& outProps)
{
    skipWhitespace(s, pos);
    if (pos >= s.size() || s[pos] != '{') return false;
    ++pos;
    skipWhitespace(s, pos);
    while (pos < s.size() && s[pos] != '}') {
        skipWhitespace(s, pos);
        std::string key;
        if (!parseString(s, pos, key)) return false;
        skipWhitespace(s, pos);
        if (pos >= s.size() || s[pos] != ':') return false;
        ++pos;
        skipWhitespace(s, pos);
        // value could be string, number, array or object (we'll capture arrays/objects as raw substrings)
        if (pos < s.size() && s[pos] == '"') {
            std::string val;
            if (!parseString(s, pos, val)) return false;
            outProps.emplace_back(key, val);
        } else if (pos < s.size() && s[pos] == '[') {
            // capture array substring from pos to matching ']'
            size_t start = pos;
            int depth = 0;
            size_t i = pos;
            while (i < s.size()) {
                if (s[i] == '[') ++depth;
                else if (s[i] == ']') {
                    --depth;
                    if (depth == 0) { ++i; break; }
                }
                ++i;
            }
            if (i > s.size()) return false;
            std::string arr = s.substr(start, i - start);
            pos = i;
            outProps.emplace_back(key, arr);
        } else {
            // number or literal: capture until comma or closing brace
            size_t start = pos;
            size_t i = pos;
            while (i < s.size() && s[i] != ',' && s[i] != '}') ++i;
            std::string token = s.substr(start, i - start);
            // trim whitespace
            size_t a = 0;
            while (a < token.size() && isspace((unsigned char)token[a])) ++a;
            size_t b = token.size();
            while (b > a && isspace((unsigned char)token[b - 1])) --b;
            std::string trimmed = token.substr(a, b - a);
            pos = i;
            outProps.emplace_back(key, trimmed);
        }
        skipWhitespace(s, pos);
        if (pos < s.size() && s[pos] == ',') { ++pos; skipWhitespace(s, pos); }
    }
    if (pos >= s.size() || s[pos] != '}') return false;
    ++pos;
    return true;
}

static bool checkJsonBalance(const std::string& s, std::string& outMessage) {
    std::vector<char> stack;
    bool inString = false;
    bool escape = false;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (inString) {
            if (escape) { escape = false; continue; }
            if (c == '\\') { escape = true; continue; }
            if (c == '"') { inString = false; continue; }
        } else {
            if (c == '"') { inString = true; continue; }
            if (c == '{' || c == '[') { stack.push_back(c); continue; }
            if (c == '}' || c == ']') {
                if (stack.empty()) {
                    outMessage = "Unexpected closing bracket at pos " + std::to_string(i);
                    return false;
                }
                char top = stack.back(); stack.pop_back();
                if ((c == '}' && top != '{') || (c == ']' && top != '[')) {
                    outMessage = "Mismatched bracket at pos " + std::to_string(i);
                    return false;
                }
            }
        }
    }
    if (inString) { outMessage = "Unterminated string in JSON (missing closing \")"; return false; }
    if (!stack.empty()) {
        std::string expected;
        for (auto it = stack.rbegin(); it != stack.rend(); ++it) {
            expected += ((*it == '{') ? '}' : ']');
        }
        outMessage = "JSON truncated: missing closing sequence \"" + expected + "\"";
        return false;
    }
    return true;
}

bool SceneEditor::loadLevel(const std::wstring& filename, std::vector<SerializedObject>& outObjects)
{
    const std::string path = wstringToUtf8(filename);
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) return false;
    std::ostringstream oss;
    oss << ifs.rdbuf();
    std::string content = oss.str();
    outObjects.clear();
    // Trim leading whitespace
    size_t pos = 0;
    skipWhitespace(content, pos);

    // Try to find a top-level "version" value if present (optional)
    int fileVersion = 1;
    size_t verPos = content.find("\"version\"");
    if (verPos != std::string::npos) {
        size_t colon = content.find(':', verPos);
        if (colon != std::string::npos) {
            size_t p = colon + 1;
            skipWhitespace(content, p);
            // parse integer
            size_t start = p;
            while (p < content.size() && isdigit((unsigned char)content[p])) ++p;
            if (p > start) {
                try { fileVersion = std::stoi(content.substr(start, p - start)); }
                catch (...) { fileVersion = 1; }
            }
        }
    }

    bool parsedAny = false;

    // Determine if file is a top-level array or an object containing "objects"
    if (pos < content.size() && content[pos] == '[') {
        // top-level array of objects (legacy)
        size_t cur = pos;
        while (true) {
            size_t objStart = content.find('{', cur);
            if (objStart == std::string::npos) break;
            size_t objEnd = content.find('}', objStart);
            if (objEnd == std::string::npos) break;
            // parse fields inside object
            size_t typePos = content.find("\"type\"", objStart);
            std::string type;
            if (typePos != std::string::npos && typePos < objEnd) {
                size_t colon = content.find(':', typePos);
                if (colon != std::string::npos && colon < objEnd) {
                    size_t tmp = colon + 1;
                    parseString(content, tmp, type);
                }
            }
            Vector3D position(0,0,0), rotation(0,0,0), scale(1,1,1);
            size_t posPos = content.find("\"position\"", objStart);
            if (posPos != std::string::npos && posPos < objEnd) {
                size_t colon = content.find(':', posPos);
                if (colon != std::string::npos && colon < objEnd) {
                    size_t p = colon + 1;
                    parseArray3(content, p, position);
                }
            }
            size_t rotPos = content.find("\"rotation\"", objStart);
            if (rotPos != std::string::npos && rotPos < objEnd) {
                size_t colon = content.find(':', rotPos);
                if (colon != std::string::npos && colon < objEnd) {
                    size_t p = colon + 1;
                    parseArray3(content, p, rotation);
                }
            }
            size_t scPos = content.find("\"scale\"", objStart);
            if (scPos != std::string::npos && scPos < objEnd) {
                size_t colon = content.find(':', scPos);
                if (colon != std::string::npos && colon < objEnd) {
                    size_t p = colon + 1;
                    parseArray3(content, p, scale);
                }
            }

            SerializedObject so;
            so.type = type;
            so.position = position;
            so.rotation = rotation;
            so.scale = scale;

            // Legacy arrays won't have components; push empty components vector.
            outObjects.push_back(so);
            parsedAny = true;
            cur = objEnd + 1;
        }
    } else {
        // try to find "objects" array in an object (current format)
        size_t objectsPos = content.find("\"objects\"");
        if (objectsPos != std::string::npos) {
            size_t colon = content.find(':', objectsPos);
            if (colon != std::string::npos) {
                size_t arrStart = content.find('[', colon);
                if (arrStart != std::string::npos) {
                    size_t cur = arrStart + 1;
                    while (true) {
                        // find next object start
                        size_t objStart = content.find('{', cur);
                        if (objStart == std::string::npos) break;
                        // find matching end for this object using simple brace matching
                        size_t i = objStart;
                        int depth = 0;
                        while (i < content.size()) {
                            if (content[i] == '{') ++depth;
                            else if (content[i] == '}') { --depth; if (depth == 0) { ++i; break; } }
                            ++i;
                        }
                        if (i > content.size()) break;
                        size_t objEnd = i - 1; // index of closing '}'
                        // parse fields inside object bounds [objStart, objEnd]
                        size_t typePos = content.find("\"type\"", objStart);
                        std::string type;
                        if (typePos != std::string::npos && typePos < objEnd) {
                            size_t colon2 = content.find(':', typePos);
                            if (colon2 != std::string::npos && colon2 < objEnd) {
                                size_t tmp = colon2 + 1;
                                parseString(content, tmp, type);
                            }
                        }
                        Vector3D position(0,0,0), rotation(0,0,0), scale(1,1,1);
                        size_t posPos = content.find("\"position\"", objStart);
                        if (posPos != std::string::npos && posPos < objEnd) {
                            size_t colon2 = content.find(':', posPos);
                            if (colon2 != std::string::npos && colon2 < objEnd) {
                                size_t p = colon2 + 1;
                                parseArray3(content, p, position);
                            }
                        }
                        size_t rotPos = content.find("\"rotation\"", objStart);
                        if (rotPos != std::string::npos && rotPos < objEnd) {
                            size_t colon2 = content.find(':', rotPos);
                            if (colon2 != std::string::npos && colon2 < objEnd) {
                                size_t p = colon2 + 1;
                                parseArray3(content, p, rotation);
                            }
                        }
                        size_t scPos = content.find("\"scale\"", objStart);
                        if (scPos != std::string::npos && scPos < objEnd) {
                            size_t colon2 = content.find(':', scPos);
                            if (colon2 != std::string::npos && colon2 < objEnd) {
                                size_t p = colon2 + 1;
                                parseArray3(content, p, scale);
                            }
                        }

                        SerializedObject so;
                        so.type = type;
                        so.position = position;
                        so.rotation = rotation;
                        so.scale = scale;

                        // Parse optional components block inside this object
                        size_t compsPos = content.find("\"components\"", objStart);
                        if (compsPos != std::string::npos && compsPos < objEnd) {
                            size_t colon3 = content.find(':', compsPos);
                            if (colon3 != std::string::npos && colon3 < objEnd) {
                                size_t arrS = content.find('[', colon3);
                                if (arrS != std::string::npos && arrS < objEnd) {
                                    size_t curComp = arrS + 1;
                                    while (true) {
                                        // find next component object
                                        size_t compStart = content.find('{', curComp);
                                        if (compStart == std::string::npos || compStart >= objEnd) break;
                                        // find matching end for component
                                        size_t j = compStart;
                                        int d = 0;
                                        while (j < content.size()) {
                                            if (content[j] == '{') ++d;
                                            else if (content[j] == '}') { --d; if (d == 0) { ++j; break; } }
                                            ++j;
                                        }
                                        if (j > content.size() || j - 1 > objEnd) break;
                                        size_t compEnd = j - 1;
                                        // inside compStart..compEnd: find "type" and optional "props"
                                        size_t ctypePos = content.find("\"type\"", compStart);
                                        std::string ctype;
                                        if (ctypePos != std::string::npos && ctypePos < compEnd) {
                                            size_t colon4 = content.find(':', ctypePos);
                                            if (colon4 != std::string::npos && colon4 < compEnd) {
                                                size_t tmp = colon4 + 1;
                                                parseString(content, tmp, ctype);
                                            }
                                        }
                                        SceneEditor::ComponentRecord crec;
                                        crec.type = ctype;
                                        // parse props
                                        size_t propsPos = content.find("\"props\"", compStart);
                                        if (propsPos != std::string::npos && propsPos < compEnd) {
                                            size_t colon5 = content.find(':', propsPos);
                                            if (colon5 != std::string::npos && colon5 < compEnd) {
                                                size_t p = colon5 + 1;
                                                // parse object of props
                                                std::vector<std::pair<std::string,std::string>> props;
                                                if (parsePropsObject(content, p, props)) {
                                                    crec.properties = props;
                                                }
                                            }
                                        }
                                        so.components.push_back(crec);
                                        parsedAny = true;
                                        curComp = compEnd + 1;
                                        // move to next component in array
                                    }
                                }
                            }
                        }

                        outObjects.push_back(so);
                        parsedAny = true;
                        cur = objEnd + 1;
                    }
                }
            }
        }
    }

    // If nothing parsed, return false
    return parsedAny;
}
