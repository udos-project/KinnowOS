#include <ctype.h>
#include <bits/features.h>

STDAPI int islower(int x)
{
    return
    ((x) == 'a' || (x) == 'b' || (x) == 'c' || (x) == 'd' || (x) == 'e'
     || (x) == 'f' || (x) == 'g' || (x) == 'h' || (x) == 'i' || (x) == 'j'
     || (x) == 'k' || (x) == 'l' || (x) == 'm' || (x) == 'n' || (x) == 'o'
     || (x) == 'p' || (x) == 'q' || (x) == 'r' || (x) == 's' || (x) == 't'
     || (x) == 'u' || (x) == 'v' || (x) == 'w' || (x) == 'x' || (x) == 'y'
     || (x) == 'z') ? 1 : 0;
}

STDAPI int isupper(int x)
{
    return
    ((x) == 'A' || (x) == 'B' || (x) == 'C' || (x) == 'D' || (x) == 'E'
     || (x) == 'F' || (x) == 'G' || (x) == 'H' || (x) == 'I' || (x) == 'J'
     || (x) == 'K' || (x) == 'L' || (x) == 'M' || (x) == 'N' || (x) == 'O'
     || (x) == 'P' || (x) == 'Q' || (x) == 'R' || (x) == 'S' || (x) == 'T'
     || (x) == 'U' || (x) == 'V' || (x) == 'W' || (x) == 'X' || (x) == 'Y'
     || (x) == 'Z') ? 1 : 0;
}

STDAPI int isdigit(int x)
{
    return
    ((x) == '0' || (x) == '1' || (x) == '2' || (x) == '3' || (x) == '4'
     || (x) == '5' || (x) == '6' || (x) == '7' || (x) == '8' || (x) == '9');
}

STDAPI int isalpha(int x)
{
    return islower(x) || isupper(x) ? 1 : 0;
}

STDAPI int isalnum(int x)
{
    return isalpha(x) || isdigit(x) ? 1 : 0;
}

STDAPI int toupper(int x)
{
    return
    (x) == 'a' ? 'A' : (x) == 'g' ? 'G' : (x) == 'm' ? 'M' : (x) == 's' ? 'S' :
    (x) == 'b' ? 'B' : (x) == 'h' ? 'H' : (x) == 'n' ? 'N' : (x) == 't' ? 'T' :
    (x) == 'c' ? 'C' : (x) == 'i' ? 'I' : (x) == 'o' ? 'O' : (x) == 'u' ? 'U' :
    (x) == 'd' ? 'D' : (x) == 'j' ? 'J' : (x) == 'p' ? 'P' : (x) == 'v' ? 'V' :
    (x) == 'e' ? 'E' : (x) == 'k' ? 'K' : (x) == 'q' ? 'Q' : (x) == 'w' ? 'W' :
    (x) == 'f' ? 'F' : (x) == 'l' ? 'L' : (x) == 'r' ? 'R' : (x) == 'x' ? 'X' :
    (x) == 'y' ? 'Y' : (x) == 'z' ? 'Z' : (x);
}

STDAPI int tolower(int x)
{
    return
    (x) == 'A' ? 'a' : (x) == 'G' ? 'g' : (x) == 'M' ? 'm' : (x) == 'S' ? 's' :
    (x) == 'B' ? 'b' : (x) == 'H' ? 'h' : (x) == 'N' ? 'n' : (x) == 'T' ? 't' :
    (x) == 'C' ? 'c' : (x) == 'I' ? 'i' : (x) == 'O' ? 'o' : (x) == 'U' ? 'u' :
    (x) == 'D' ? 'd' : (x) == 'J' ? 'j' : (x) == 'P' ? 'p' : (x) == 'V' ? 'v' :
    (x) == 'E' ? 'e' : (x) == 'K' ? 'k' : (x) == 'Q' ? 'q' : (x) == 'W' ? 'w' :
    (x) == 'F' ? 'f' : (x) == 'L' ? 'l' : (x) == 'R' ? 'r' : (x) == 'X' ? 'x' :
    (x) == 'Y' ? 'y' : (x) == 'Z' ? 'z' : (x);
}

STDAPI int isspace(int x)
{
    return x == ' ' || x == '\t' || x == '\v' || x == '\r' || x == '\n' || x == '\f' ? 1 : 0;
}

STDAPI int ispunct(int x)
{
    return x == '.' || x == ',' || x == ';' || x == ':' ? 1 : 0;
}

STDAPI int isxdigit(int x)
{
    x = tolower(x);
    if(isdigit(x) || x == 'A' || x == 'B' || x == 'C' || x == 'D' || x == 'E' || x == 'F') {
        return 1;
    }
    return 0;
}
