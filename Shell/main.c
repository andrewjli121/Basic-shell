strdup_every_other_char(const char* s) {
    int length = 0;
    const char* b = s;
    while(*s != NULL) {
        length += 1;
        s += 1;
    }
    char* result = new char[(length + 1) / 2];
    s = b;
    for(int i = 0; i < (length + 1) / 2; i++) {
        result[i] = *s;
        s += 2;
    }
};

int main() {

    printf(strdup_every_other_char("house"));
}