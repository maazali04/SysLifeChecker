#pragma once

#include <wx/font.h>
#include <string>

class FontManager
{
public:

    static bool Initialize();

    static wxFont Regular(int size);
    static wxFont Medium(int size);
    static wxFont SemiBold(int size);
    static wxFont Bold(int size);

private:

    static bool m_Initialized;

    static constexpr const char* FontFamily = "Inter";

    static const std::string FontFolder;

    static const std::string RegularPath;
    static const std::string MediumPath;
    static const std::string SemiBoldPath;
    static const std::string BoldPath;
};