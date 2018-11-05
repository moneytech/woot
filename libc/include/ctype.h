#ifndef CTYPE_H
#define CTYPE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int isdigit(int c);
int tolower(int c);
int toupper(int c);
int isspace(int c);
int isxdigit(int c);
int isalpha(int c);
int isupper(int c);
int isalnum(int c);
int isprint(int c);
int isgraph(int c);
int iscntrl(int c);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // CTYPE_H
