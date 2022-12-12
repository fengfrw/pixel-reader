#include "../xhtml_parser.h"

#include <gtest/gtest.h>

static void ASSERT_TOKENS_EQ(const std::vector<DocToken> &actual_tokens, const std::vector<DocToken> &expected_tokens)
{
    auto actual_it = actual_tokens.begin();
    auto expected_it = expected_tokens.begin();
    while (actual_it != actual_tokens.end() && expected_it != expected_tokens.end())
    {
        const auto &actual = *actual_it;
        const auto &expected = *expected_it;
    
        EXPECT_EQ(actual.type, expected.type);
        EXPECT_EQ(actual.address, expected.address);
        EXPECT_EQ(actual.text, expected.text);
    
        ASSERT_EQ(actual, expected);
    
        ++actual_it;
        ++expected_it;
    }

    ASSERT_EQ(actual_tokens.size(), expected_tokens.size());
    ASSERT_EQ(actual_tokens, expected_tokens);
}

static std::vector<DocToken> _parse_xhtml_tokens(const char *xml)
{
    std::vector<DocToken> tokens;
    std::unordered_map<std::string, DocAddr> ids;
    parse_xhtml_tokens(xml, "", 0, tokens, ids);
    return tokens;
}

TEST(XHTML_PARSER, basic_text_in_valid_xhtml)
{
    const char *xml = (
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>"
        "<!DOCTYPE html>"
        "<html xmlns=\"http://www.w3.org/1999/xhtml\" xmlns:epub=\"http://www.idpf.org/2007/ops\" xml:lang=\"en\" lang=\"en\">"
        "    <head>\n"
        "        <title>Title</title>\n"
        "    </head>\n"
        "    <body>\n"
        "        Text\n"
        "    </body>\n"
        "</html>"
    );
  
    std::vector<DocToken> expected_tokens {
        {TokenType::Text, 0, "Text "}
    };
  
    ASSERT_TOKENS_EQ(
        _parse_xhtml_tokens(xml),
        expected_tokens
    );
}

TEST(XHTML_PARSER, whitespace_compaction)
{
    const char *xml = (
        "<html><body>"
        "  This  <i>  has  </i>  some  <i>extra</i>  white<i> <i/>space  "
        "</body></html>"
    );
  
    std::vector<DocToken> expected_tokens {
        {TokenType::Text,  0, "This " },
        {TokenType::Text,  5, "has "  },
        {TokenType::Text,  9, "some " },
        {TokenType::Text, 14, "extra" },
        {TokenType::Text, 20, " white"},
        {TokenType::Text, 26, " "     },
        {TokenType::Text, 28, "space "}
    };
  
    ASSERT_TOKENS_EQ(
        _parse_xhtml_tokens(xml),
        expected_tokens
    );
}

TEST(XHTML_PARSER, line_break)
{
    const char *xml = (
        "<html><body>"
        "<div>Line 1</div>"
        "<div>Line 2</div>"
        "</body></html>"
    );
  
    std::vector<DocToken> expected_tokens {
        {TokenType::Text,       1, "Line 1"},
        {TokenType::TextBreak,  6, ""      },
        {TokenType::Text,       8, "Line 2"},
        {TokenType::TextBreak, 13, ""      }
    };
  
    ASSERT_TOKENS_EQ(
        _parse_xhtml_tokens(xml),
        expected_tokens
    );
}

TEST(XHTML_PARSER, section_compaction)
{
    const char *xml = (
        "<html><body>"
        "  <p>"
        "    <p>"
        "      <div>"
        "         Some text."
        "      </div>"
        "    </p>"
        "    <p>"
        "       Some more."
        "    </p>"
        "  </p>"
        "</body></html>"
    );
  
    std::vector<DocToken> expected_tokens {
        {TokenType::Text,     3, "Some text. "},
        {TokenType::Section, 12, ""           },
        {TokenType::Text,    15, "Some more. "},
        {TokenType::Section, 24, ""           }
    };
  
    ASSERT_TOKENS_EQ(
        _parse_xhtml_tokens(xml),
        expected_tokens
    );
}

TEST(XHTML_PARSER, capture_ids)
{
    const char *xml = (
        "<html><body>"
        "<p id=\"id1\">text1</p>"
        "<p id=\"id2\">text2</p>"
        "</body></html>"
    );

    std::unordered_map<std::string, DocAddr> expected_ids {
        {"id1", 1},
        {"id2", 8},
    };
  
    std::vector<DocToken> tokens;
    std::unordered_map<std::string, DocAddr> ids;
    ASSERT_TRUE(parse_xhtml_tokens(xml, "", 0, tokens, ids));

    ASSERT_EQ(expected_ids, ids);
}