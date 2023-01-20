// ******************************* //
//            @COPYLEFT            //
//       ALL WRONGS RESERVED       //
// ******************************* //

#pragma once

#include <string>
#include <vector>

namespace cowircd
{
    class irc_message
    {
    public:
        static std::vector<std::string> split(const std::string& str);
        static bool validate_command(std::string& command);
        static bool parse(const std::string& str, irc_message& out_msg);

    private:
        std::string command;

        bool has_prefix_value;
        std::string prefix;

        std::vector<std::string> params;
        bool last_param_is_trailing;

    public:
        irc_message();
        irc_message(const std::string& command);

        std::string& operator[](int i);
        const std::string& operator[](int i) const;

        const std::string& get_command() const;

        bool has_prefix() const;
        const std::string& get_prefix() const;
        void set_prefix(const std::string& prefix);
        void reset_prefix();

        std::size_t size_param() const;
        void reserve_param(std::size_t n);
        void add_param(const std::string& param);
        void remove_all_param();

        std::string to_string() const;
    };
}
