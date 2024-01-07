
#include <unistd.h>

#include "print.h"

#include <stdarg.h>
#include <string.h>

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
                    char digits[19];
                    long my_int = va_arg(argp, long);
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
