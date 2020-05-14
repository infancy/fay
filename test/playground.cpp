#include <fay/core/fay.h>
#include <fay/core/any.h>

int main(int argc, char** argv)
{
    google::InitGoogleLogging(argv[0]);

    #pragma region playground

    std::cout << fay::any::requires_dynamic<bool>();

    #pragma endregion playground

    return 0;
}