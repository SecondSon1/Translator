namespace color {
    // Formatting
    std::wostream& reset(std::wostream& out) {
        out << "\033[0m";
        return out;
    }

    std::wostream& bright(std::wostream& out) {
        out << "\033[1m";
        return out;
    }

    std::wostream& underline(std::wostream& out) {
        out << "\033[4m";
        return out;
    }

    std::wostream& flashing(std::wostream& out) {
        out << "\033[5m";
        return out;
    }

    // Foreground colors
    std::wostream& black(std::wostream& out) {
        out << "\033[30m";
        return out;
    }

    std::wostream& red(std::wostream& out) {
        out << "\033[31m";
        return out;
    }

    std::wostream& green(std::wostream& out) {
        out << "\033[32m";
        return out;
    }

    std::wostream& yellow(std::wostream& out) {
        out << "\033[33m";
        return out;
    }

    std::wostream& blue(std::wostream& out) {
        out << "\033[34m";
        return out;
    }

    std::wostream& purple(std::wostream& out) {
        out << "\033[35m";
        return out;
    }

    std::wostream& cyan(std::wostream& out) {
        out << "\033[36m";
        return out;
    }

    std::wostream& white(std::wostream& out) {
        out << "\033[37m";
        return out;
    }

    // Background colors
    namespace background {
        std::wostream &black(std::wostream &out) {
            out << "\033[40m";
            return out;
        }

        std::wostream &red(std::wostream &out) {
            out << "\033[41m";
            return out;
        }

        std::wostream &green(std::wostream &out) {
            out << "\033[42m";
            return out;
        }

        std::wostream &yellow(std::wostream &out) {
            out << "\033[43m";
            return out;
        }

        std::wostream &blue(std::wostream &out) {
            out << "\033[44m";
            return out;
        }

        std::wostream &purple(std::wostream &out) {
            out << "\033[45m";
            return out;
        }

        std::wostream &cyan(std::wostream &out) {
            out << "\033[46m";
            return out;
        }

        std::wostream &white(std::wostream &out) {
            out << "\033[47m";
            return out;
        }
    }
}
