#ifndef JDA_NUMBER_H
#define JDA_NUMBER_H

typedef struct jda_number {
    long double real_value;

    /* Used for composed complex numbers */
    long double imaginary_value;

    /* Used for big numbers, for example 10^10^10^1000
     * isn't quite representable on normal notation so we
     * just assume the number is elevated to the nth power
     * like this */
    long double power;
} jda_number_t;

#endif
