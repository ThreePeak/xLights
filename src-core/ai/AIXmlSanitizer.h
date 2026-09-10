#pragma once

#include <string>
#include <sstream>

namespace xLights::AI {

class AIXmlSanitizer {
public:
    /**
     * @brief Sanitizes/escapes text for safe inclusion into XML attributes and elements.
     * Escapes &, <, >, ", and '.
     */
    static std::string XmlEscape(const std::string& input) {
        std::string output;
        output.reserve(input.size() + 16);
        for (char c : input) {
            switch (c) {
                case '&':  output.append("&amp;"); break;
                case '<':  output.append("&lt;"); break;
                case '>':  output.append("&gt;"); break;
                case '\"': output.append("&quot;"); break;
                case '\'': output.append("&apos;"); break;
                default:   output.push_back(c); break;
            }
        }
        return output;
    }

    /**
     * @brief Unescapes basic XML entities.
     */
    static std::string XmlUnescape(const std::string& input) {
        std::string output;
        output.reserve(input.size());
        for (size_t i = 0; i < input.size(); ++i) {
            if (input[i] == '&') {
                if (input.compare(i, 5, "&amp;") == 0) {
                    output.push_back('&');
                    i += 4;
                } else if (input.compare(i, 4, "&lt;") == 0) {
                    output.push_back('<');
                    i += 3;
                } else if (input.compare(i, 4, "&gt;") == 0) {
                    output.push_back('>');
                    i += 3;
                } else if (input.compare(i, 6, "&quot;") == 0) {
                    output.push_back('\"');
                    i += 5;
                } else if (input.compare(i, 6, "&apos;") == 0) {
                    output.push_back('\'');
                    i += 5;
                } else {
                    output.push_back(input[i]);
                }
            } else {
                output.push_back(input[i]);
            }
        }
        return output;
    }
};

} // namespace xLights::AI
