#include <stdio.h>

#include "buffer_copy.h"
#include "HalideBuffer.h"
#include "HalideRuntimeCuda.h"
#include "HalideRuntimeOpenCL.h"

using namespace Halide::Runtime;

#if (defined(TEST_CUDA) || defined(TEST_OPENCL))
int main(int argc, char **argv) {
    const halide_device_interface_t *dev = nullptr;
#ifdef TEST_CUDA
    dev = halide_cuda_device_interface();
#else
    dev = halide_opencl_device_interface();
#endif

    // Test a buffer-copy to a device buffer from a cropped host buffer.
    {
        Buffer<int> input(128, 128);
        input.fill([&](int x, int y) {return x + 10*y;});
        Buffer<int> out(64, 64);
        out.set_min(32, 32);
        Buffer<int> in_crop = input.cropped(0, 32, 64).cropped(1, 32, 64);

        halide_buffer_copy(nullptr, in_crop, dev, out);

        out.copy_to_host();

        out.for_each_value([&](int a, int b) {
                if (a != b) {
                    printf("Copying a crop failed\n");
                    exit(-1);
                }
            }, in_crop);
    }

    // Test a buffer-copy to a host buffer from a device buffer.
    {
        Buffer<int> input(128, 128);
        input.fill([&](int x, int y) {return x + 10*y;});
        Buffer<int> out(64, 64);
        out.set_min(32, 32);
        Buffer<int> in_crop = input.cropped(0, 32, 64).cropped(1, 32, 64);

        // Move the crop to the device, and damage the host data.
        in_crop.copy_to_device(dev);
        in_crop.set_device_dirty();
        in_crop.fill(0);
        in_crop.set_host_dirty(false);

        halide_buffer_copy(nullptr, in_crop, nullptr, out);

        in_crop.copy_to_host();

        out.for_each_value([&](int a, int b) {
                if (a != b) {
                    printf("Copying a crop failed\n");
                    exit(-1);
                }
            }, in_crop);
    }

    // Then a test of the pipeline that uses it in complicated ways
    if (1) {
        Buffer<float> input(128, 128);
        Buffer<float> output(128, 128);
        input.fill([&](int x, int y) {return (float)(x + y);});

        int result = buffer_copy(input, dev, output);

        if (result != 0) {
            printf("error code: %d\n", result);
        }

        output.for_each_element([&](int x, int y) {
                if (output(x, y) != input(x, y) + 4) {
                    printf("output(%d, %d) = %f intead of %f\n",
                           x, y, output(x, y), input(x, y) + 4);
                    exit(-1);
                }
            });
    }

    printf("Success!\n");
    return 0;
}

#else

int main(int argc, char **argv) {
    printf("Skipping test for non-cuda target\n");
    return 0;
}

#endif
