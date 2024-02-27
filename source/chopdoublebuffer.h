#include <vector>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>


class ChopDoubleBuffer 
{
public:
	ChopDoubleBuffer() = default;
	ChopDoubleBuffer(const ChopDoubleBuffer&) = delete;
	ChopDoubleBuffer& operator=(const ChopDoubleBuffer&) = delete;
	ChopDoubleBuffer(ChopDoubleBuffer&&) = delete;
	ChopDoubleBuffer& operator=(ChopDoubleBuffer&&) = delete;

	void write(const float* const* channelData, const char* const* names, int32_t channelCount, uint32_t valueCount)
	{
		int bufferIndex = activeBuffer.load(std::memory_order_acquire) ^ 1;
		dataBuffers[bufferIndex].resize(static_cast<size_t>(channelCount * valueCount));
		namesBuffers[bufferIndex].resize(static_cast<size_t>(channelCount));

		for (uint32_t i = 0; i < channelCount; i++)
		{
			for (uint32_t j = 0; j < valueCount; j++)
			{
				dataBuffers[bufferIndex][static_cast<size_t>(i * valueCount + j)] = channelData[i][j];
			}
			namesBuffers[bufferIndex][i] = names[i];
		}

		notify();
	}


	void read(std::vector<float>& channelData, std::vector<std::string>& names)
	{
		std::unique_lock<std::mutex> lock(mutex_);
		cv_.wait(lock, [this] { return ready; }); // Wait until data is ready
		int bufferIndex = activeBuffer.load(std::memory_order_acquire);
		channelData = dataBuffers[bufferIndex];
		names = namesBuffers[bufferIndex];

		ready = false; // Reset ready state after reading
	}

	void swapBuffers() 
	{
		activeBuffer.fetch_xor(1, std::memory_order_release);
	}

private:
	std::vector<float> dataBuffers[2];
	std::vector<std::string> namesBuffers[2];
	std::atomic<int> activeBuffer{ 0 }; // Index of the buffer that is ready for reading
	std::mutex mutex_;
	std::condition_variable cv_;
	bool ready{ false }; // Indicates if the buffer is ready to be swapped/read

	void notify()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		ready = true; // Mark as ready for reading
		cv_.notify_one(); // Notify the reading thread
	}

};
