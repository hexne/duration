import std;
import args_parser;
import time;

std::filesystem::path get_config_path() {
    std::filesystem::path config_path = std::getenv("HOME");
    return config_path / "config.json";
}

void print_help() {
    std::println("{:*^40}", "");
    std::println("-g -gui, 后台运行带GUI的统计程序\n"
                        "-h --help, 查看帮助\n"
                        "-l --list, 历史记录"
                );
    std::println("{:*^40}", "");
}

void print_list() {

}

void gui_show() {

}

int main(int argc, char *argv[]) {
    ArgsParser args_parser(argc, argv);
    args_parser.add_args("-h", "--help", print_help);
    args_parser.add_args("-l", "--list", print_list);
    args_parser.add_args("-g", "--gui", gui_show);
    args_parser.parser();
    return 0;
}