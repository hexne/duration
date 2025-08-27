#include <QApplication>
#include <QTimer>
#include <QPixmap>
#include <QPainter>
#include <QSystemTrayIcon>

import std;
import args_parser;
import time;

struct Duration {
    Time begin_time;
    Time end_time;
    bool is_finish{};
    Time count() const {
        if (is_finish)
            return end_time - begin_time;
        else
            return Time::now() - begin_time;
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

auto& get_max_duration() {
    return *std::ranges::max_element(duration_list, [&](const Duration& left, const Duration &right) -> bool {
        return left.count() < right.count();
    });
}

auto &get_cur_duration() {
    return *std::ranges::find_if(duration_list, [&](const Duration& duration) -> bool {
        return !duration.is_finish;
    });
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
            duration.is_finish ? duration.end_time.get_string() : [&] -> std::string {
                return std::vformat(
                    std::format("{{:^{}}}", duration.end_time.get_string().size()),
                   std::make_format_args("now"));
            }(),
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

inline int angle(float p) {
    return -p * 360 * 16;
}

QPixmap draw() {
    auto cur_count = get_cur_duration();
    Time end_time;

    auto count_time = end_time - cur_count.begin_time;

    int h = count_time.get_hour();
    int m = count_time.get_minute();
    int s = count_time.get_second();

    QPixmap ret(32, 32);
    ret.fill(Qt::transparent);
    QPainter painter(&ret);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform); //抗锯齿和使用平滑转换算法

    int pen_width = 4;
    painter.setPen(QPen(QColor(166, 77, 255), pen_width));

    // second
    painter.drawArc(pen_width / 2, pen_width / 2, 32 - pen_width, 32 - pen_width, 90 * 16, angle(s * 1.f / 60));

    painter.setPen(QPen(QColor(51, 153, 255), pen_width));
    // minute
    painter.drawArc(pen_width * 1.5f, pen_width * 1.5f, 32 - pen_width * 3, 32 - pen_width * 3, 90 * 16, angle(m * 1.f / 60));

    painter.setPen(QPen(Qt::black, pen_width));
    painter.drawArc(pen_width * 2.5f, pen_width * 2.5f, 32 - pen_width * 5, 32 - pen_width * 5, 90 * 16, angle(h * 1.f / 24));

    return ret;
}

void gui_show(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QSystemTrayIcon icon;
    icon.setIcon(QIcon(draw()));
    icon.show();

    QTimer timer;
    auto cur_count = get_cur_duration();
    QObject::connect(&timer, &QTimer::timeout, [&] {
        icon.setIcon(QIcon(draw()));
        auto &&count_time = cur_count.count();
        auto message = std::format("{} day, {}", count_time.count<std::chrono::days>(), count_time.get_clock_string());
        icon.setToolTip(message.data());
        icon.show();
    });
    timer.start(500);

    app.exec();
}

int main(int argc, char *argv[]) {

    load();

    ArgsParser args_parser(argc, argv);
    args_parser.add_args("-h", "--help", print_help);
    // args_parser.add_args("-l", "--list", [] { print_list(false); });
    args_parser.add_args("-l", "--list", std::bind_front(print_list, false));
    args_parser.add_args("--sort-list", std::bind_front(print_list, true));
    args_parser.add_args("-g", "--gui", std::bind_front(gui_show, argc, argv));
    args_parser.add_args("reset");
    args_parser.parser();
    if (args_parser["reset"].enable) {
        reset(args_parser["reset"].value);
    }

    return 0;
}