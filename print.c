
#include <unistd.h>

#include "print.h"

#include <stdarg.h>
#include <string.h>

void print_long(long my_long)
{
    // print a long
    // max long value: 9223372036854775807 which is 19 digits
    char digits[19];
    int digit_idx = sizeof(digits) / sizeof(char) - 1;
    for (;; --digit_idx)
    {
        char ch = (my_long % 10) + '0';
        digits[digit_idx] = ch;
        if (my_long < 10)
            break;
        my_long /= 10;
    }
    write(STDOUT_FILENO, &digits[digit_idx], sizeof(digits) / sizeof(char) - digit_idx);
}

void print(const char *format, ...) // TODO fix so less writes are done
{
    va_list argp;
    va_start(argp, format);

    int normal_text_from = 0;
    for (int i = 0;; ++i)
    {
        const char ch = format[i];
        if (ch == '\0')
        {

            int len = i - normal_text_from;
            if (len > 0)
                write(STDOUT_FILENO, &format[normal_text_from], len);
            break;
        }
        if ('%' == ch)
        {
            // write text so far
            {
                int len = i - normal_text_from;
                if (len > 0)
                    write(STDOUT_FILENO, &format[normal_text_from], len);
            }
            normal_text_from = i + 2;
            const char ch = format[++i];
            switch (ch)
            {
            case 'l': {
                normal_text_from += 1;
                const char ch = format[++i];
                switch (ch)
                {
                case 'd': {
                    // print a long
                    // max long value: 9223372036854775807 which is 19 digits
                    long my_long = va_arg(argp, long);
                    print_long(my_long);
                }
                break;
                }
            }
            break;
            case 'd': {
                // print an int
                // max int value: 2147483647 which is 10 digits
                char digits[10];
                int my_int = va_arg(argp, int);
                int digit_idx = sizeof(digits) / sizeof(char) - 1;
                for (;; --digit_idx)
                {
                    char ch = (my_int % 10) + '0';
                    digits[digit_idx] = ch;
                    if (my_int < 10)
                        break;
                    my_int /= 10;
                }
                write(STDOUT_FILENO, &digits[digit_idx], sizeof(digits) / sizeof(char) - digit_idx);
            }
            break;
            case 's': {
                // string
                const char *str = va_arg(argp, const char *);
                unsigned long len = strlen(str);
                write(STDOUT_FILENO, str, len);
            }
            break;
            case 'f': {
                // floating point
                double my_float = va_arg(argp, double);
                long long_part = (long)my_float;
                double fractional_part = my_float - (double)long_part;

                print_long(long_part);
                write(STDOUT_FILENO, ".", 1);
                fractional_part *= 1000000;
                long fractional_part_long = (long)fractional_part;
                print_long(fractional_part_long);
                
            }
            break;
            case 'c': {
                // print char
                char ch = va_arg(argp, int);
                write(STDOUT_FILENO, &ch, 1);
            }
            break;
            }
        }
    }

    va_end(argp);
}
