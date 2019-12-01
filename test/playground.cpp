#include <fay/container/array.h>

int main(int argc, char** argv)
{
    google::InitGoogleLogging(argv[0]);

    fay::array<int, 4> a{ 0, 0, 0, 0, 0 };

    return 0;
}