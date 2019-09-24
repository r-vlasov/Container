class A { 
        public:
        static int b;
};

int A::b;
int main() {
        A::b = 2;
        return 0;
}
