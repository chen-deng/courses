struct String {
    string str<20>;
};

program STRINGOPS_PROG {
    version STRINGOPS_VERS {
        String UPPER(String) = 1;
    } = 1;
} = 0x20000001;