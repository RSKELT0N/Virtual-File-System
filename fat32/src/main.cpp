#include "../include/terminal.h"

#include <dirent.h>

void load_disks() {
   std::string path = "disks/";
   std::vector<std::string> func(3);
   func[0] = "ifs";
   func[1] = "add";

   struct dirent *entry;
   DIR *dir = opendir(path.c_str());

   while((entry = readdir(dir)) != NULL) {
       if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
           continue;


       func[2] = entry->d_name;
       VFS::get_vfs()->control_vfs(func);
   }

	closedir(dir);
}

int main(int argc, char** argv) {
    load_disks();
    
    terminal* term = new terminal();
    term->run();
    delete term;
    
    return 0;
}
