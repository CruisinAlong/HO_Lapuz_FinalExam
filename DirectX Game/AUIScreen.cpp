#include "AUIScreen.h"

AUIScreen::AUIScreen(String name) : name(name) {}
AUIScreen::~AUIScreen() {}

AUIScreen::String AUIScreen::getName() { return name; }
