#include "test_framework.h"
#include "teleios/teleios.h"

void test_number(void) {
    TEST_SUITE_BEGIN("Number");

    TLAllocator* allocator = global->allocator;

    // ============================================
    // Digit Counting
    // ============================================

    TEST_BEGIN("tl_number_signed_digits");
    {
        ASSERT_EQ(1, tl_number_signed_digits(0));
        ASSERT_EQ(1, tl_number_signed_digits(1));
        ASSERT_EQ(1, tl_number_signed_digits(9));
        ASSERT_EQ(2, tl_number_signed_digits(10));
        ASSERT_EQ(2, tl_number_signed_digits(99));
        ASSERT_EQ(3, tl_number_signed_digits(100));
        ASSERT_EQ(3, tl_number_signed_digits(999));

        // Negative numbers
        ASSERT_EQ(2, tl_number_signed_digits(-1));   // "-1" has 2 chars
        ASSERT_EQ(3, tl_number_signed_digits(-10));  // "-10" has 3 chars
        ASSERT_EQ(4, tl_number_signed_digits(-100)); // "-100" has 4 chars
    }
    TEST_END();

    TEST_BEGIN("tl_number_unsigned_digits");
    {
        ASSERT_EQ(1, tl_number_unsigned_digits(0));
        ASSERT_EQ(1, tl_number_unsigned_digits(1));
        ASSERT_EQ(1, tl_number_unsigned_digits(9));
        ASSERT_EQ(2, tl_number_unsigned_digits(10));
        ASSERT_EQ(2, tl_number_unsigned_digits(99));
        ASSERT_EQ(3, tl_number_unsigned_digits(100));
        ASSERT_EQ(10, tl_number_unsigned_digits(1000000000));
    }
    TEST_END();

    // ============================================
    // Number to String (Signed)
    // ============================================

    TEST_BEGIN("tl_number_i8_to_char");
    {
        TLString* str = tl_number_i8_to_char(allocator, 42, 10);
        ASSERT_NOT_NULL(str);
        ASSERT_STR_EQ("42", tl_string_cstr(str));
        tl_string_destroy(str);

        str = tl_number_i8_to_char(allocator, -42, 10);
        ASSERT_NOT_NULL(str);
        ASSERT_STR_EQ("-42", tl_string_cstr(str));
        tl_string_destroy(str);

        str = tl_number_i8_to_char(allocator, 0, 10);
        ASSERT_NOT_NULL(str);
        ASSERT_STR_EQ("0", tl_string_cstr(str));
        tl_string_destroy(str);
    }
    TEST_END();

    TEST_BEGIN("tl_number_i32_to_char");
    {
        TLString* str = tl_number_i32_to_char(allocator, 12345, 10);
        ASSERT_NOT_NULL(str);
        ASSERT_STR_EQ("12345", tl_string_cstr(str));
        tl_string_destroy(str);

        str = tl_number_i32_to_char(allocator, -12345, 10);
        ASSERT_NOT_NULL(str);
        ASSERT_STR_EQ("-12345", tl_string_cstr(str));
        tl_string_destroy(str);
    }
    TEST_END();

    TEST_BEGIN("tl_number_i64_to_char");
    {
        TLString* str = tl_number_i64_to_char(allocator, 9223372036854775807LL, 10);
        ASSERT_NOT_NULL(str);
        ASSERT_STR_EQ("9223372036854775807", tl_string_cstr(str));
        tl_string_destroy(str);
    }
    TEST_END();

    // ============================================
    // Number to String (Unsigned)
    // ============================================

    TEST_BEGIN("tl_number_u8_to_char");
    {
        TLString* str = tl_number_u8_to_char(allocator, 255, 10);
        ASSERT_NOT_NULL(str);
        ASSERT_STR_EQ("255", tl_string_cstr(str));
        tl_string_destroy(str);

        str = tl_number_u8_to_char(allocator, 0, 10);
        ASSERT_NOT_NULL(str);
        ASSERT_STR_EQ("0", tl_string_cstr(str));
        tl_string_destroy(str);
    }
    TEST_END();

    TEST_BEGIN("tl_number_u32_to_char");
    {
        TLString* str = tl_number_u32_to_char(allocator, 4294967295U, 10);
        ASSERT_NOT_NULL(str);
        ASSERT_STR_EQ("4294967295", tl_string_cstr(str));
        tl_string_destroy(str);
    }
    TEST_END();

    // ============================================
    // Number to String (Different Bases)
    // ============================================

    TEST_BEGIN("number_to_char_base_16");
    {
        TLString* str = tl_number_u8_to_char(allocator, 255, 16);
        ASSERT_NOT_NULL(str);
        ASSERT_STR_EQ("ff", tl_string_cstr(str));
        tl_string_destroy(str);

        str = tl_number_u32_to_char(allocator, 0xDEADBEEF, 16);
        ASSERT_NOT_NULL(str);
        ASSERT_STR_EQ("deadbeef", tl_string_cstr(str));
        tl_string_destroy(str);
    }
    TEST_END();

    TEST_BEGIN("number_to_char_base_2");
    {
        TLString* str = tl_number_u8_to_char(allocator, 10, 2);
        ASSERT_NOT_NULL(str);
        ASSERT_STR_EQ("1010", tl_string_cstr(str));
        tl_string_destroy(str);

        str = tl_number_u8_to_char(allocator, 255, 2);
        ASSERT_NOT_NULL(str);
        ASSERT_STR_EQ("11111111", tl_string_cstr(str));
        tl_string_destroy(str);
    }
    TEST_END();

    TEST_BEGIN("number_to_char_base_8");
    {
        TLString* str = tl_number_u8_to_char(allocator, 64, 8);
        ASSERT_NOT_NULL(str);
        ASSERT_STR_EQ("100", tl_string_cstr(str));
        tl_string_destroy(str);
    }
    TEST_END();

    // ============================================
    // String to Number (Unsigned)
    // ============================================

    TEST_BEGIN("tl_number_u8_from_string");
    {
        TLString* str = tl_string_create(allocator, "255");
        ASSERT_EQ(255, tl_number_u8_from_string(str));
        tl_string_destroy(str);

        str = tl_string_create(allocator, "0");
        ASSERT_EQ(0, tl_number_u8_from_string(str));
        tl_string_destroy(str);
    }
    TEST_END();

    TEST_BEGIN("tl_number_u16_from_string");
    {
        TLString* str = tl_string_create(allocator, "65535");
        ASSERT_EQ(65535, tl_number_u16_from_string(str));
        tl_string_destroy(str);
    }
    TEST_END();

    TEST_BEGIN("tl_number_u32_from_string");
    {
        TLString* str = tl_string_create(allocator, "4294967295");
        ASSERT_EQ(4294967295U, tl_number_u32_from_string(str));
        tl_string_destroy(str);
    }
    TEST_END();

    TEST_BEGIN("tl_number_u64_from_string");
    {
        TLString* str = tl_string_create(allocator, "18446744073709551615");
        ASSERT_EQ(18446744073709551615ULL, tl_number_u64_from_string(str));
        tl_string_destroy(str);
    }
    TEST_END();

    // ============================================
    // String to Number (Signed)
    // ============================================

    TEST_BEGIN("tl_number_i8_from_string");
    {
        TLString* str = tl_string_create(allocator, "127");
        ASSERT_EQ(127, tl_number_i8_from_string(str));
        tl_string_destroy(str);

        str = tl_string_create(allocator, "-128");
        ASSERT_EQ(-128, tl_number_i8_from_string(str));
        tl_string_destroy(str);
    }
    TEST_END();

    TEST_BEGIN("tl_number_i16_from_string");
    {
        TLString* str = tl_string_create(allocator, "32767");
        ASSERT_EQ(32767, tl_number_i16_from_string(str));
        tl_string_destroy(str);

        str = tl_string_create(allocator, "-32768");
        ASSERT_EQ(-32768, tl_number_i16_from_string(str));
        tl_string_destroy(str);
    }
    TEST_END();

    TEST_BEGIN("tl_number_i32_from_string");
    {
        TLString* str = tl_string_create(allocator, "2147483647");
        ASSERT_EQ(2147483647, tl_number_i32_from_string(str));
        tl_string_destroy(str);

        str = tl_string_create(allocator, "-2147483648");
        ASSERT_EQ(-2147483648, tl_number_i32_from_string(str));
        tl_string_destroy(str);
    }
    TEST_END();

    TEST_BEGIN("tl_number_i64_from_string");
    {
        TLString* str = tl_string_create(allocator, "9223372036854775807");
        ASSERT_EQ(9223372036854775807LL, tl_number_i64_from_string(str));
        tl_string_destroy(str);
    }
    TEST_END();

    // ============================================
    // String to Number (Float)
    // ============================================

    TEST_BEGIN("tl_number_f32_from_string");
    {
        TLString* str = tl_string_create(allocator, "3.14159");
        ASSERT_FLOAT_EQ(3.14159f, tl_number_f32_from_string(str), 0.0001f);
        tl_string_destroy(str);

        str = tl_string_create(allocator, "-2.5");
        ASSERT_FLOAT_EQ(-2.5f, tl_number_f32_from_string(str), 0.0001f);
        tl_string_destroy(str);

        str = tl_string_create(allocator, "0.0");
        ASSERT_FLOAT_EQ(0.0f, tl_number_f32_from_string(str), 0.0001f);
        tl_string_destroy(str);
    }
    TEST_END();

    TEST_BEGIN("tl_number_f64_from_string");
    {
        TLString* str = tl_string_create(allocator, "3.141592653589793");
        ASSERT_FLOAT_EQ(3.141592653589793, tl_number_f64_from_string(str), 0.0000001);
        tl_string_destroy(str);

        str = tl_string_create(allocator, "-123.456789");
        ASSERT_FLOAT_EQ(-123.456789, tl_number_f64_from_string(str), 0.0001);
        tl_string_destroy(str);
    }
    TEST_END();

    // ============================================
    // Utility Functions
    // ============================================

    TEST_BEGIN("tl_number_next_power_of_2");
    {
        ASSERT_EQ(1, tl_number_next_power_of_2(0));
        ASSERT_EQ(1, tl_number_next_power_of_2(1));
        ASSERT_EQ(2, tl_number_next_power_of_2(2));
        ASSERT_EQ(4, tl_number_next_power_of_2(3));
        ASSERT_EQ(4, tl_number_next_power_of_2(4));
        ASSERT_EQ(8, tl_number_next_power_of_2(5));
        ASSERT_EQ(8, tl_number_next_power_of_2(7));
        ASSERT_EQ(8, tl_number_next_power_of_2(8));
        ASSERT_EQ(16, tl_number_next_power_of_2(9));
        ASSERT_EQ(256, tl_number_next_power_of_2(200));
        ASSERT_EQ(1024, tl_number_next_power_of_2(1000));
    }
    TEST_END();

    // ============================================
    // Edge Cases
    // ============================================

    TEST_BEGIN("number_conversion_roundtrip");
    {
        // Test that converting to string and back yields the same number
        i32 original = -12345;
        TLString* str = tl_number_i32_to_char(allocator, original, 10);
        i32 result = tl_number_i32_from_string(str);
        ASSERT_EQ(original, result);
        tl_string_destroy(str);
    }
    TEST_END();

    TEST_BEGIN("float_conversion_special_values");
    {
        TLString* str = tl_string_create(allocator, "1.0");
        ASSERT_FLOAT_EQ(1.0f, tl_number_f32_from_string(str), 0.0001f);
        tl_string_destroy(str);

        str = tl_string_create(allocator, ".5");
        ASSERT_FLOAT_EQ(0.5f, tl_number_f32_from_string(str), 0.0001f);
        tl_string_destroy(str);
    }
    TEST_END();

    TEST_SUITE_END();
}
