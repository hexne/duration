import std;
import args_parser;
import time;

struct Duration {
    Time begin_time;
    Time end_time;
    bool is_finish{};
    Time count() const {
        return end_time - begin_time;
    }
};
std::vector<Duration> duration_list;


std::filesystem::path get_config_file_path() {
    std::filesystem::path config_path = std::getenv("HOME");
    return config_path / ".config/count_time/config.txt";
}

void load() {
    std::fstream file(get_config_file_path());
    std::string line;
    while (std::getline(file, line)) {
        Duration duration;
        if (line.length() <= 20) {
            duration.begin_time = Time(line.substr(0, 19));
            duration.is_finish = false;;
        }
        else {
            std::string begin_str = line.substr(0, 19);
            std::string end_str = line.substr(20, 19);
            duration.begin_time = Time(begin_str);
            duration.end_time = Time(end_str);
            duration.is_finish = true;
        }
        duration_list.emplace_back(std::move(duration));
    }
}

void print_help() {
    std::println("{:*^40}", "");
    std::println("-g -gui, 后台运行带GUI的统计程序\n"
                        "-h --help, 查看帮助\n"
                        "-l --list, 历史记录"
                );
    std::println("{:*^40}", "");
}

void print_list(bool sort = false) {
    if (sort) {
        std::ranges::sort(duration_list, [&](const Duration& left, const Duration &right) -> bool {
            return left.count() > right.count();
        });
    }
    auto max = std::ranges::max_element(duration_list, [&](const Duration& left, const Duration &right) -> bool {
        return left.count() < right.count();
    });
    auto str = std::format("| {} --> {} | {} day {} |",
        max->begin_time.get_string(),
        max->end_time.get_string(),
        max->count().count<std::chrono::days>(),
        max->count().get_clock_string());
    for (int i = 0;i < str.size(); ++i) {
        std::cout << '=';
    }
    std::cout << '\n';




    for (auto &duration : duration_list) {
        auto str = std::format("| {} --> {} | {} day {} |\n",
            duration.begin_time.get_string(),
            duration.end_time.get_string(),
            duration.count().count<std::chrono::days>(),
            duration.count().get_clock_string());
        std::cout << str;
    }
    for (int i = 0;i < str.size(); ++i) {
        std::cout << '=';
    }
    std::cout << '\n';
}

void reset(const std::string &time_str) {
    Time time;
    try {
        time = Time(time_str);
    }
    catch (...) {
        std::cerr << "错误的日期格式 : " << time_str << std::endl;
    }
    std::fstream file(get_config_file_path(), std::ios::app);
    file << time.get_string() << "\n";
    file << time.get_string() + " ";
}

void gui_show() {


}

int main(int argc, char *argv[]) {

    load();

    ArgsParser args_parser(argc, argv);
    args_parser.add_args("-h", "--help", print_help);
    args_parser.add_args("-l", "--list", print_list);
    args_parser.add_args("-g", "--gui", gui_show);
    args_parser.add_args("reset");
    args_parser.parser();
    if (args_parser["reset"].enable) {
        reset(args_parser["reset"].value);
    }

    return 0;
}