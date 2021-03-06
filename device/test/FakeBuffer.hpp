#ifndef EMBMESSENGER_TEST_FAKEBUFFER_HPP
#define EMBMESSENGER_TEST_FAKEBUFFER_HPP

#include "EmbMessenger/IBuffer.hpp"

#include <cstdint>
#include <vector>

namespace emb
{
    namespace device
    {
        namespace test
        {
            class FakeBuffer : public shared::IBuffer
            {
            protected:
                std::vector<uint8_t> host;
                std::vector<uint8_t> device;
                int hostMessages = 0;
                bool readCrc = false;
                bool appendCrc = true;
                bool validCrc = true;

            public:
                /**
                 * @brief Appends a message to the host buffer, auto adds crc.
                 *   Used for adding a message to be read by the device.
                 *
                 * @param message Message to append
                 */
                void addHostMessage(std::vector<uint8_t>&& message);

                /**
                 * @brief Checks if the message is in the device buffer, auto adds crc.
                 *   Used for checking if the device wrote what was expected.
                 *   Removes the number of elements in message from the device buffer.
                 *
                 * @param message Message to check for in the host buffer
                 *
                 * @returns True if the message was in the host buffer
                 */
                bool checkDeviceBuffer(std::vector<uint8_t>&& message);

                bool buffersEmpty();

                void writeCrc(const bool value);
                void writeValidCrc(const bool value);

                void print();

                virtual void writeByte(const uint8_t byte) override;
                virtual uint8_t peek() const override;
                virtual uint8_t readByte() override;
                virtual bool empty() const override;
                virtual size_t size() const override;
                virtual uint8_t messages() const override;
                virtual void update() override;
                virtual void zero() override;
            };
        }  // namespace test
    }  // namespace device
}  // namespace emb

#endif  // EMBMESSENGER_TEST_FAKEBUFFER_HPP
