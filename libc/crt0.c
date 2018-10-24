extern int main(int argc, char *argv[]);

void _start()
{
    char *arg0 = "test";
    char *argv[] = { arg0 };
    main(1, argv);
}
