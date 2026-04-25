#include "vision/QualityRecognizer.h"

#include <cassert>

int main()
{
    bbae::QualityRecognizer recognizer;
    assert(recognizer.recognizePlaceholder().empty());
    return 0;
}

