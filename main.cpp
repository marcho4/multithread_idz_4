#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <random>
#include <fstream>

// Config
#define DEFAULT_LOG_PATH "./logs.txt"
#define MIN_SMOKER 1
#define MAX_SMOKER 10
#define MIN_PUB 1
#define MAX_PUB 10

class SmokingManager {
    std::vector<std::mutex> queue_mutexes;
    std::vector<std::condition_variable> conditions;
    std::vector<std::queue<std::string>> component_queues;
    std::vector<std::string> components = {"paper", "tabak", "spichki"};
    std::random_device rd;
    std::ofstream log_file;
    int smoker_min_time;
    int smoker_max_time;
    int publisher_min_time;
    int publisher_max_time;

public:
    SmokingManager(const std::string& file_name, int smoker_min, int smoker_max, int publisher_min, int publisher_max)
        : queue_mutexes(3), conditions(3), component_queues(3),
          smoker_min_time(smoker_min), smoker_max_time(smoker_max),
          publisher_min_time(publisher_min), publisher_max_time(publisher_max) {
        log_file.open(file_name);
        if (!log_file.is_open()) {
            throw std::runtime_error("Failed to open log file");
        }
    }

    ~SmokingManager() {
        if (log_file.is_open()) {
            log_file.close();
        }
    }

    void main_cycle() {
        std::mt19937 gen(rd());
        std::uniform_int_distribution distrib(0, 2);
        std::uniform_int_distribution sleep_time(publisher_min_time, publisher_max_time);

        while (true) {
            int seconds = sleep_time(gen);
            std::this_thread::sleep_for(std::chrono::seconds(seconds));

            int randomNumber = distrib(gen);
            std::string missing_component = components[randomNumber];

            // Поместить компонент в очередь соответствующего курильщика
            {
                std::lock_guard<std::mutex> lock(queue_mutexes[randomNumber]);
                component_queues[randomNumber].push(missing_component);
                std::cout << "Component missing: " << missing_component << "\n";
                log_file << "Component missing: " << missing_component << "\n";
                log_file.flush();
            }

            // Уведомить соответствующего курильщика
            conditions[randomNumber].notify_one();
        }
    }

    void worker(int id) {
        std::mt19937 gen(rd());
        std::uniform_int_distribution smoke_time(smoker_min_time, smoker_max_time);

        std::string own_component = components[id];

        while (true) {
            std::unique_lock<std::mutex> lock(queue_mutexes[id]);

            // Ожидание, пока в очереди не появится новый компонент
            conditions[id].wait(lock, [this, id]() {
                return !component_queues[id].empty();
            });

            // Извлекаем компонент из очереди
            std::string component = component_queues[id].front();
            component_queues[id].pop();
            lock.unlock(); // Освобождаем мьютекс очереди

            // Курим
            int smoke_sec = smoke_time(gen);

            std::cout << "Smoker with " << own_component << " started smoking for " << smoke_sec << " seconds\n";
            log_file << "Smoker with " << own_component << " started smoking for " << smoke_sec << " seconds\n";
            log_file.flush();
            std::this_thread::sleep_for(std::chrono::seconds(smoke_sec));

            std::cout << "Smoker with " << own_component << " finished smoking\n";
            log_file << "Smoker with " << own_component << " finished smoking\n";
            log_file.flush();
        }
    }
};

void printUsage(const std::string& programName) {
    std::cerr << "Usage: " << programName << " [file_path]" << std::endl;
    std::cerr << "If no file_path is provided, default path will be used: "
              << DEFAULT_LOG_PATH << std::endl;
}

int main(int argc, char* argv[]) {
    // Ввод через консоль
    // std::string file_name;
    // int smoker_min, smoker_max, publisher_min, publisher_max;
    //
    // std::cout << "Enter the name of the log file: ";
    // std::cin >> file_name;
    //
    // std::cout << "Enter smoker min time: ";
    // std::cin >> smoker_min;
    //
    // std::cout << "Enter smoker max time: ";
    // std::cin >> smoker_max;
    //
    // std::cout << "Enter publisher min time: ";
    // std::cin >> publisher_min;
    //
    // std::cout << "Enter publisher max time: ";
    // std::cin >> publisher_max;
    // Путь к логу по умолчанию
    std::string file_path = DEFAULT_LOG_PATH;

    // Обработка аргументов командной строки
    if (argc > 2) {
        // Слишком много аргументов
        printUsage(argv[0]);
        return 1;
    }
    else if (argc == 2) {
        // Проверка флага help
        if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") {
            printUsage(argv[0]);
            return 0;
        }

        // Установка пользовательского пути
        file_path = argv[1];
    }

    try {
        SmokingManager manager(file_path, MIN_SMOKER, MAX_SMOKER, MIN_PUB, MAX_PUB);

        // Запуск потоков
        std::thread main_thread(&SmokingManager::main_cycle, &manager);
        std::thread worker1(&SmokingManager::worker, &manager, 0);
        std::thread worker2(&SmokingManager::worker, &manager, 1);
        std::thread worker3(&SmokingManager::worker, &manager, 2);

        main_thread.join();
        worker1.join();
        worker2.join();
        worker3.join();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
