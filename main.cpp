#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <random>

class SmokingManager {
    std::vector<std::mutex> queue_mutexes;
    std::vector<std::condition_variable> conditions;
    std::vector<std::queue<std::string>> component_queues;
    std::vector<std::string> components = {"paper", "tabak", "spichki"};
    std::random_device rd;

public:
    SmokingManager() : queue_mutexes(3), conditions(3), component_queues(3) {}

    void main_cycle() {
        std::mt19937 gen(rd());
        std::uniform_int_distribution distrib(0, 2);
        std::uniform_int_distribution sleep_time(1, 10);

        while (true) {
            int seconds = sleep_time(gen);
            std::this_thread::sleep_for(std::chrono::seconds(seconds));

            int randomNumber = distrib(gen);
            std::string missing_component = components[randomNumber];

            // Поместить компонент в очередь соответствующего курильщика
            {
                std::lock_guard<std::mutex> lock(queue_mutexes[randomNumber]);
                component_queues[randomNumber].push(missing_component);
                std::cout << "Generated missing component: " << missing_component << std::endl;
            }

            // Уведомить соответствующего курильщика
            conditions[randomNumber].notify_one();
        }
    }

    void worker(int id) {
        std::mt19937 gen(rd());
        std::uniform_int_distribution smoke_time(1, 10);

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
            std::cout << "Worker " << own_component << " started smoking for " << smoke_sec << " seconds\n";
            std::this_thread::sleep_for(std::chrono::seconds(smoke_sec));
            std::cout << "Worker " << own_component << " finished smoking\n";
        }
    }
};

int main() {
    SmokingManager manager;

    // Запуск потоков
    std::thread main_thread(&SmokingManager::main_cycle, &manager);
    std::thread worker1(&SmokingManager::worker, &manager, 0);
    std::thread worker2(&SmokingManager::worker, &manager, 1);
    std::thread worker3(&SmokingManager::worker, &manager, 2);

    main_thread.join();
    worker1.join();
    worker2.join();
    worker3.join();

    return 0;
}
