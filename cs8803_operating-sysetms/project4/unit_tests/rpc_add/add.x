struct IntPair {
    int a;
    int b;
};

program ADD_PROG {
    version ADD_VERS {
        int ADD(IntPair) = 1;
    } = 1;
} = 0x23451111;