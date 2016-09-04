/*BK auto login */
#include <unistd.h>
int main() {
 execlp("login","login","-f","root",0);
 return 0;
}
