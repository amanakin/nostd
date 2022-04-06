#include "include/nostd/array/array.h"
#include "include/nostd/storage/storage.h"

#include <iostream>

int main() {
    nostd::Array<int, nostd::storage::DynamicStorage> array(10);
    array[0] = 1;
    array.push_back(100);

    for (size_t idx = 0; idx < array.size(); ++idx) {
        std::cout << array[idx] << '\n';
    }
}

