#pragma once

#include <iostream>

#define IS_SUCCESS(expr) do { std::cout << "[OK] " #expr << std::endl; } while(0)
#define IS_FAILURE(expr) do { std::cout << "[FAILURE] " #expr << std::endl; } while(0)
#define ASSERT_TRUE(expr) do { if (expr) { IS_SUCCESS(expr); } else { IS_FAILURE(expr); } } while(0)
#define ASSERT_FALSE(expr) do { if (expr) { IS_FAILURE(expr); } else { IS_SUCCESS(expr); } } while(0)

