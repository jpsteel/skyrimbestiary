#include "Sync.h"
#include "Scaleform.h"
#include "HUDWidget.h"
#include <thread>
#include <chrono>


std::queue<std::string> entryQueue;
bool isDisplayingEntry = false;
std::mutex entryMutex;

void ProcessQueue() {
    while (true) {
        std::unique_lock<std::mutex> lock(entryMutex);
        auto* ui = RE::UI::GetSingleton();
        auto* tasks = SKSE::GetTaskInterface();

        if (ui && tasks && !entryQueue.empty() && !isDisplayingEntry && !ui->GameIsPaused()) {
            std::string variant = entryQueue.front();
            entryQueue.pop();

            lock.unlock();

            isDisplayingEntry = true;

            tasks->AddUITask([variant] {
                Scaleform::HUDWidget::DisplayEntry(variant);
                });

            std::this_thread::sleep_for(std::chrono::seconds(4));

            isDisplayingEntry = false;
        } else {
            lock.unlock();
            std::this_thread::sleep_for(
                std::chrono::milliseconds(100));
        }
    }
}