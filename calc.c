// Adil Yuksel 241ADB145
// Repository: calculator_lab
// Compile with: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm
// Resources used: course lab PDF; calc-tests.zip; OpenAI ChatGPT/Codex (conversation history to be supplied).
//ChatGPT conservation link: https://chatgpt.com/s/cx_6ade82dc51f4819196bf64d6420185c8
// Compile: gcc -O2 -Wall -Wextra -std=c17 -o calc calc.c -lm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>

//AI (ChatGPT/Codex), prompt translated from Turkish:
// AI prompt
// How can I keep whole numbers and decimal numbers together
// in struct number by using a type flag
// number storage
// I keep whole and decimal values together here
// the flag tells me which value I should use
struct number {
    int is_real;
    long long whole;
    double real;
};

char *text;
size_t textLen;
size_t pos;
size_t line_end;
size_t errorPos;

// shared positions
// I keep the input and reading positions available here
// every parser step uses the same current place
// AI prompt
// How can I use get_sum get_product get_power and get_atom
// to split a mathematical expression parser by operation priority
// parser steps
// I split the parser into small levels here
// each level handles its own operation priority
struct number get_sum(void);
struct number get_product(void);
struct number get_power(void);
struct number get_atom(void);

void memoryError(void) {
    // memory check
    // I stop the program when memory cannot be reserved
    // this keeps me from using an empty pointer
    fprintf(stderr, "calc: not enough memory\n");
    exit(1);
}
void set_error(size_t place)
{
    // AI prompt
    // How can I use set_error to save only the first error position
    // while parsing an expression
    // Do not replace the first error with a later error.
    if (!errorPos) {
        errorPos = place;
    }
}

struct number zeroNum(void)
{
    // starting value
    // I start every new result with zero values
    // the calculation fills the right field later
    struct number number;

    number.is_real = 0;
    number.whole = 0;
    number.real = 0.0;
    return number;
}

double as_real(struct number number) {
  // AI prompt
  // How can I use as_real to convert my number structure
  // to double only when I need it
  // value conversion
  // I return the decimal value when it already exists
  // otherwise I convert the whole value for the calculation
  if (number.is_real) {
        return number.real;
  }
    return (double)number.whole;
}

void skip_spaces(void)
{
    // space handling
    // I move past spaces and tabs before reading more
    // this lets the input have a natural layout
    while(pos < line_end && (text[pos] == ' ' || text[pos] == '\t'))
        pos++;
}

struct number calculate(struct number left, struct number right, char op,
                        size_t op_pos, size_t right_pos)
{
    // AI prompt
    // How can I use calculate for addition subtraction multiplication
    // and division with both whole and decimal numbers
    // basic calculation
    // I handle the main math operations in one place
    // I also check for division by zero here
    struct number answer;
    double a;
    double b;
    answer = zeroNum();
    if (errorPos) {
        return answer;
    }
    // zero division
    // I check the right value before doing division
    // I save its position when the value is zero
    if (op == '/' &&
        ((right.is_real == 1 && right.real == 0.0) ||
         (right.is_real == 0 && right.whole == 0))) {
        set_error(right_pos);
        return answer;}
    // decimal operation
    // I use decimal math when either side is decimal
    // I also check that the final value is valid
    if (left.is_real || right.is_real) {
        a = as_real(left);
        b = as_real(right);
        answer.is_real = 1;

        if (op == '+') {     answer.real = a + b;} 
        else if (op == '-') {
            answer.real = a - b;} 
        else if (op == '*') {
            answer.real = a * b;} 
            else if (op == '/') {answer.real = a / b;} 
            else if (op == 'p') {answer.real = pow(a, b);}
        if (!isfinite(answer.real)) { set_error(op_pos); }
        return answer;}

    // whole operation
    // I keep the result as a whole number here
    // this avoids changing its type without a reason
    if (op == '+') {answer.whole = left.whole + right.whole;} 
    else if (op == '-') {answer.whole = left.whole - right.whole;} 
    else if (op == '*') {answer.whole = left.whole * right.whole;} 
    else if (op == '/') {answer.whole = left.whole / right.whole;}
    return answer;
}

struct number calcPower(struct number left, struct number right,
 size_t op_pos, size_t right_pos) {
    // AI prompt
    // How can I use calcPower to handle special power cases
    // like zero one minus one and negative exponents
    // power calculation
    // I use decimal math when one value is decimal
    // I use a loop when both values are whole numbers
    struct number answer;
    answer = zeroNum();
if (errorPos != 0) {return answer;}
   if (left.is_real) {
    return calculate(left, right, 'p', op_pos, right_pos);}
if (right.is_real) {
    return calculate(left, right, 'p', op_pos, right_pos);}
    // negative power
    // I reject a negative whole exponent in this mode
    // I point to the exponent as the bad input
    if (right.whole < 0) {
        set_error(right_pos);
        return answer;}
    // These bases can have a very large exponent without a long loop.
    if (right.whole == 0) {
        answer.whole = 1;
        return answer;
    }
    if (left.whole == 0 || left.whole == 1) {
        answer.whole = left.whole;
        return answer;
    }
    if (left.whole == -1) {
        if (right.whole % 2 == 0) {
            answer.whole = 1;} 
        else {
            answer.whole = -1;}
        return answer;}
    // repeated power
    // I multiply the base once for every exponent step
    // I keep the running result in the answer
    answer.whole = 1;
    for (long long i = 0; i < right.whole; i++) {
        answer.whole = answer.whole * left.whole;}
    return answer;}
struct number get_atom(void)
{
    // AI prompt
    // How can I use get_atom to read an expression in parentheses
    // and check that its closing parenthesis exists
    // smallest expression
    // I read one number or one expression in brackets
    // the other parser steps build on this result
    struct number number;
    size_t start;
    size_t scan;
    double whole_real;
    double frac;
    double place;

    number = zeroNum();
    skip_spaces();
    if (pos >= line_end) {
        set_error(line_end + 1);
        return number;}
    // An atom is either a number or an expression inside parentheses.
    if (text[pos] == '(') {
        pos++;
        number = get_sum();
        if (errorPos) {
            return number;}
        skip_spaces();
        if (pos >= line_end || text[pos] != ')') {
            set_error(pos + 1);
            return number;}
        pos++;
        return number;}
    if (text[pos] < '0' || text[pos] > '9') {
        set_error(pos + 1);
        return number;}
    // AI prompt
    // How can I use get_atom to read whole and decimal digits
    // from a character array without scanf
    // number scan
    // I first find where the whole number part ends
    // this also shows me if a decimal point follows
    start = pos;
    scan = pos;
    while (scan < line_end && text[scan] >= '0' && text[scan] <= '9') {
        scan++;}
    // decimal reading
    // I build both sides of the decimal value by hand
    // each new fraction digit gets a smaller place value
    if (scan < line_end && text[scan] == '.') {
        number.is_real = 1;
        whole_real = 0.0;
        while (pos < scan) {
            whole_real = whole_real * 10.0 + (text[pos] - '0');
            pos++;}
        pos++;
        if (pos >= line_end || text[pos] < '0' || text[pos] > '9') {
            set_error(scan + 1);
            return number;}
        frac = 0.0;
        place = 0.1;
        while (pos < line_end && text[pos] >= '0' && text[pos] <= '9') {
            frac = frac + (text[pos] - '0') * place;
            place = place / 10.0;
            pos++;
        }
        number.real = whole_real + frac;
        if (!isfinite(number.real)) {
            set_error(start + 1);
        }
    } else {
        // whole reading
        // I build the whole number one digit at a time
        // each old digit moves one place to the left
        while (pos < scan) {
            number.whole = number.whole * 10 + (text[pos] - '0');
            pos++;
        }}
    return number;}
struct number get_power(void)
{
 // AI prompt
 // How can I use recursion in get_power to make the double star
 // power operator group from the right
 // power priority
 // I read powers before multiplication and addition
 // I read the right side again for chained powers
 struct number left;
 struct number right;
 size_t opPos;
 size_t rpos;
    left = get_atom();
    skip_spaces();
    // power sign
    // I look for two star signs next to each other
    // I save both operand positions for error reporting
    if (!errorPos && pos + 1 < line_end &&
        text[pos] == '*' && text[pos + 1] == '*') {
        opPos = pos + 1;
        pos += 2;
        skip_spaces();
        rpos = pos + 1;
        right = get_power();
        left = calcPower(left, right, opPos, rpos); }
    return left;}
struct number get_product(void)
{
    // AI prompt
    // How can I use get_product and get_sum to make multiplication
    // and division run before addition and subtraction
    // product priority
    // I handle multiplication and division at this level
    // both sides may already contain power expressions
    struct number left;
    struct number right;
    char op;
    size_t op_pos;
    size_t right_pos;

    left = get_power();
    skip_spaces();
    // product loop
    // I keep reading while multiplication or division follows
    // every new result becomes the next left value
    while (!errorPos && pos < line_end &&
           ((text[pos] == '*' &&
             !(pos + 1 < line_end && text[pos + 1] == '*')) ||
            text[pos] == '/')) {
        op = text[pos];
        op_pos = pos + 1;
        pos++;
        skip_spaces();
        right_pos = pos + 1;
        right = get_power();
        left = calculate(left, right, op, op_pos, right_pos);
        skip_spaces();
    }
    return left;
}
struct number get_sum(void)
{
    // sum priority
    // I handle addition and subtraction after other operations
    // this keeps the normal math order
    struct number left;
    struct number right;
    char op;
    size_t op_pos;
    size_t right_pos;
    left = get_product();
    skip_spaces();
    // sum loop
    // I keep reading while addition or subtraction follows
    // I stop when the line or another expression level begins
    while (errorPos == 0 && pos < line_end &&
           (text[pos] == '+' || text[pos] == '-')) {
        op = text[pos];
        op_pos = pos + 1;
        pos++;
        skip_spaces();
        right_pos = pos + 1;
        right = get_product();
        left = calculate(left, right, op, op_pos, right_pos);
        skip_spaces();
    }
    return left;}
char *readFile(char *filename)
{
    // AI prompt
    // How can I use readFile to measure a file and read all of it
    // into dynamically allocated memory
    // file loading
    // I read the whole input file into memory here
    // I add a final zero so I can use it as text
    FILE *in;
    char *data;
    long measured;
    size_t size;
    size_t amount;
    // open input
    // I open the source in binary mode for exact file size
    // I return nothing when the file cannot be opened
    in = fopen(filename, "rb");
    if (in == NULL) {
        fprintf(stderr, "calc: cannot open %s\n", filename);
        return NULL;}
    if (fseek(in, 0, SEEK_END) != 0) {
        fprintf(stderr, "calc: cannot read %s\n", filename);
        fclose(in);
        return NULL;}
    // file size
    // I move to the end to measure the complete input
    // I then return to the start before reading it
    measured = ftell(in);
    if (measured < 0 || fseek(in, 0, SEEK_SET) != 0) {
        fprintf(stderr, "calc: cannot read %s\n", filename);
        fclose(in);
        return NULL;}
    size = measured;
    data = malloc(size + 1);
    if (data == NULL) {
        fclose(in);
        memoryError();}
    // file content
    // I copy every byte into the reserved memory
    // I reject the file when the full read is not possible
    amount = fread(data, 1, size, in);
    if (amount != size) {
        fprintf(stderr, "calc: cannot read %s\n", filename);
        free(data);
        fclose(in);
        return NULL; }
    fclose(in);
    data[size] = '\0';
    textLen = size;
    return data;
}
int evaluate_file(FILE *out, int print_now)
{
    // AI prompt
    // How can I use evaluate_file and processFile to check the input first
    // and write results only when every line is valid
    // line evaluation
    // I calculate every useful line one at a time
    // I skip empty lines and comment lines
    size_t line_start;
    size_t first;
    struct number answer;
    errorPos = 0;
    line_start = 0;
    // line search
    // I find the end of one line before parsing it
    // I stop the full pass when an error appears
    while (line_start < textLen && !errorPos) {
        line_end = line_start;
        while (line_end < textLen && text[line_end] != '\n') {
            line_end++;}
        first = line_start;
        while (first < line_end && (text[first] == ' ' || text[first] == '\t')) {
            first++;}
        // useful line
        // I ignore blank lines and lines starting with a hash
        // I send every other line to the expression parser
        if (first < line_end && text[first] != '#') {
            pos = line_start;
            answer = get_sum();
            skip_spaces();
            if (!errorPos && pos != line_end) {
                set_error(pos + 1);
            }
            // result writing
            // I print decimals and whole numbers in their own format
            // I only write during the second clean pass
            if (!errorPos && print_now) {
                int written;
                if (answer.is_real) {
                    written = fprintf(out, "%.15g\n", answer.real);
                } else {
                    written = fprintf(out, "%lld\n", answer.whole);}
                if (written < 0) {
                    return 0;}
                }
            }
        if (line_end < textLen) {
            line_start = line_end + 1;
        } else {
            line_start = textLen;}
        }
    return 1;}
char *baseName(char *path, int remove_txt) {
    // AI prompt
    // How can I use baseName join_path outputName and default_folder
    // to create output paths filenames and folder names from an input path
    size_t len;
    size_t start;
    size_t base_len;
    char *base;
    // Find the part after the last slash.
    len = strlen(path);
    while (len > 1 && path[len - 1] == '/') {
      len--;}
    start = len;
    while (start > 0 && path[start - 1] != '/') {
        start--;}
    // extension removal
    // I remove the txt ending only when the caller asks
    // I keep every other part of the base name
    if (remove_txt && len - start >= 4 &&
        strncmp(path + len - 4, ".txt", 4) == 0) {
        len = len - 4;}
    base_len = len - start;
    base = malloc(base_len + 1);
    if (base == NULL) {
        memoryError();}
    memcpy(base, path + start, base_len);
    base[base_len] = '\0';
    return base;}
char *join_path(char *folder, char *filename) {
    // full path
    // I join the folder and filename with a slash
    // this gives me the exact place for the file
    size_t size;
    char *path;
    size = strlen(folder) + strlen(filename) + 2;
    path = malloc(size);
    if (path == NULL) {
        memoryError();}
    strcpy(path, folder);
    strcat(path, "/");
    strcat(path, filename);
    return path;
}
char *outputName(char *input)
{
    // output name
    // I build the required result filename here
    // I include my name and student number in it
    char *base;
    char *name;
    size_t size;
    base = baseName(input, 1);
    size = strlen(base) + strlen("adil") + strlen("yuksel") +
           strlen("241ADB145") + 8;
    name = malloc(size);
    if (name == NULL) {
        free(base);
        memoryError();}
    snprintf(name, size, "%s_%s_%s_%s.txt",
             base, "adil", "yuksel", "241ADB145");
    free(base);
    return name;
}
char *default_folder(char *input, int is_folder)
{
    // default output
    // I make an output folder name from the input name
    // I use it only when no folder was given by the user
    char *base;
    char *folder;
    size_t size;
    base = baseName(input, is_folder == 0);
    size = strlen(base) + strlen("adil") + strlen("241ADB145") + 3;
    folder = malloc(size);
    if (folder == NULL) {
        free(base);
        memoryError();
    }
    snprintf(folder, size, "%s_%s_%s", base, "adil", "241ADB145");
    free(base);
    return folder;
}
int makeFolder(char *folder) {
    // folder setup
    // I create the output folder when it is missing
    // I also accept it when it already exists
    struct stat info;
    if (mkdir(folder, 0755) == 0) {
        return 1;
    }
    if (errno == EEXIST && stat(folder, &info) == 0 && S_ISDIR(info.st_mode)) {
        return 1;
    }
    fprintf(stderr, "calc: cannot create output folder %s\n", folder);
    return 0;}
int processFile(char *filename, char *outdir)
{
    // one file job
    // I check the file before writing any result
    // I write the first error or all answers
    char *output_name;
    char *output_path;
    FILE *out;
    int write_ok;
    size_t first_error;
    text = readFile(filename);
    if (text == NULL) {
        return 0;}
    // First pass finds an error. The second pass writes only if there is none.
    evaluate_file(NULL, 0);
    first_error = errorPos;
    output_name = outputName(filename);
    output_path = join_path(outdir, output_name);
    out = fopen(output_path, "w");
    if (out == NULL) {
        fprintf(stderr, "calc: cannot write %s\n", output_path);
        free(text);
        free(output_name);
        free(output_path);
        return 0;}
    if (first_error) {
        // error output
        // I write the first bad position when parsing failed
        // otherwise I run the clean pass and write answers
        write_ok = fprintf(out, "ERROR:%zu\n", first_error) >= 0;}
         else {write_ok = evaluate_file(out, 1);}
    if (fclose(out) != 0) {write_ok = 0;}
    if (!write_ok) {fprintf(stderr, "calc: cannot write %s\n", output_path);}
    free(text);
    free(output_name);
    free(output_path);
    return write_ok;}
int is_txt(char *name)
{ size_t len = strlen(name);
    // text filter
    // I accept names that end with the txt extension
    // short names cannot contain that ending
    if (len < 4) { return 0;}
    return strcmp(name + len - 4, ".txt") == 0;}
int collectFiles(char *dirname, char ***files, size_t *count)
{
    // AI prompt
    // How can I use collectFiles and free_file_list to collect txt files
    // grow the list with realloc and release its memory later
    // folder scan
    // I collect regular txt files from this folder
    // I grow the list when its space is full
    DIR *folder;
    struct dirent *entry;
    struct stat info;
    char *path;
    char **new_files;
    size_t size;

    folder = opendir(dirname);
    if (folder == NULL) {
        fprintf(stderr, "calc: cannot open folder %s\n", dirname);
        return 0;
    }

    size = 10;
    *count = 0;
    *files = malloc(size * sizeof(char *));
    if (*files == NULL) {
        closedir(folder);
        memoryError();
    }

    // Only regular .txt files directly inside the folder are used.
    while ((entry = readdir(folder)) != NULL) {
        if (is_txt(entry->d_name)) {
            path = join_path(dirname, entry->d_name);
            if (stat(path, &info) != 0) {
                fprintf(stderr, "calc: cannot check %s\n", path);
                free(path);
                closedir(folder);
                return 0;
            }
            if (S_ISREG(info.st_mode)) {
                // list growth
                // I double the storage when the list becomes full
                // I keep all paths already collected in the new block
                if (*count >= size) {
                    size *= 2;
                    new_files = realloc(*files, size * sizeof(char *));
                    if (new_files == NULL) {
                        closedir(folder);
                        memoryError();}
                    *files = new_files;}
                (*files)[*count] = path;
                (*count)++;} 
                else {free(path);} 
            } 
        }
    closedir(folder);
    return 1;}

void free_file_list(char **files, size_t count) {
    // list cleanup
    // I release every saved path after the work is done
    // I release the main list after that
    if (files == NULL) {
        return;}
    for (size_t j = 0; j < count; j++) {
      free(files[j]);}
    free(files);}
int usage(void)
{
    // usage message
    // I show the accepted command shape after bad input
    // I return one to report that the run failed
    fprintf(stderr,
            "Usage: calc [-d DIR | --dir DIR] "
            "[-o OUTDIR | --output-dir OUTDIR] [input.txt]\n");
    return 1;}
int main(int argc, char *argv[])
{
    // AI prompt
    // How can I use argv in main to read file mode folder mode
    // and output folder options by hand
    // program flow
    // I read the options and choose file or folder mode
    // I prepare the output folder and process the input
    char *filename;
    char *dirname;
    char *outdir;
    char *default_outdir;
    char **files;
    size_t file_count;
    struct stat info;
    int ok;

    filename = NULL;
    dirname = NULL;
    outdir = NULL;
    default_outdir = NULL;
    files = NULL;
    file_count = 0;

    // option reading
    // I check each command option from left to right
    // I save the chosen input and output locations
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dir") == 0) {
            if (dirname != NULL || i + 1 >= argc) {
                return usage();}
            dirname = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "-o") == 0 ||
                   strcmp(argv[i], "--output-dir") == 0) {
            if (outdir != NULL || i + 1 >= argc) {
                return usage();}
            outdir = argv[i + 1];
            i++;}
             else if (argv[i][0] == '-' || filename != NULL) {
            return usage();} 
        else {filename = argv[i];}
    }
    if ((filename == NULL && dirname == NULL) ||
        (filename != NULL && dirname != NULL)) {
        return usage();}
    // input check
    // I allow either one file or one folder but not both
    // I also make sure a selected file really exists
    if (filename != NULL) {
        if (stat(filename, &info) != 0 || !S_ISREG(info.st_mode)) {
            fprintf(stderr, "calc: input file does not exist: %s\n", filename);
            return 1;}
    } 
    else {
        if (!collectFiles(dirname, &files, &file_count)) {
            free_file_list(files, file_count);
            return 1;}
    }
    if (outdir == NULL) {
        if (filename != NULL) {default_outdir = default_folder(filename, 0);} 
        else { default_outdir = default_folder(dirname, 1);}
        outdir = default_outdir;}
    if (!makeFolder(outdir)) {
        free(default_outdir);
        free_file_list(files, file_count);
        return 1;}
    ok = 1;
    if (filename != NULL) {
        if (!processFile(filename, outdir)) {
            ok = 0;
        }
    } else {
        for (size_t j = 0; j < file_count; j++) {
            if (!processFile(files[j], outdir)) {
                ok = 0;
            }
        }
    }
    // final cleanup
    // I release temporary names and the collected file list
    // I return success only when every file was processed
    free(default_outdir);
    free_file_list(files, file_count);
    if (ok) {
        return 0;
    }
    return 1;
}
