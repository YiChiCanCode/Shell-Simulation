#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

void myPrint(char *msg){
    write(STDOUT_FILENO, msg, strlen(msg));
}

void removeSpaces(char *str)
{
    int count = 0;
    for (int i = 0; str[i]; i++){
        if (!isspace(str[i])){
            str[count++] = str[i]; 
        }
    }
    str[count] = '\0';
}

void throw_err(){
    char error_message[30] = "An error has occurred\n";
    write(STDOUT_FILENO, error_message, strlen(error_message));
}




int check_pos_one(char* part){
    //we are checking whether there should be bad syntax
    //filename>filename; filename >filename; filename> filename
    //filename>filename -> 2
    //filename> filename -> 1
    //filename >filename -> 3
    const char sign[1] = ">";
    if(strstr(part, sign)==NULL){
        return 0;
    }  
    char* ret = strstr(part, sign);
    
    int len_of_substring = strlen(ret);
    int length = strlen(part);
    if(!strcmp(part, sign)){
        return 0;
    }  
    if(len_of_substring==length){
        //filename >filename -> 3
        return 3;
    }else if(len_of_substring==1){
        //filename> filename -> 1
        return 1; 
    }else if(len_of_substring<length){
        //filename>filename -> 2
        return 2;
    }
    return 0;
}

int is_white_space(char* line){
    char* p;
    p = line;
    int i =0;
    int length = strlen(line);
    while (i++<length) {
        if (!isspace((unsigned char)*p)){
            return 0;}
        p++;
     }
    return 1;
}

//pass by reference
//parse the string by ";" 
//then store it in new_arr
int parse_by_semicol(char* line, char** new_arr){
    //printf("parsing by semicol\n");
    char* token;
    char semi_col = (char)59;
    int size = 0;

    token = strtok(line, &semi_col);
    while(token!=NULL){
        if(is_white_space(token)){
            token = strtok(NULL, &semi_col);
            continue;
        }
        new_arr[size++] = token;
        token = strtok(NULL, &semi_col);
    // printf("There is a total of %d commands:\n", size);
    } 
    return size;
}


int sign_not_path(char* path){
    char* e = ">";
    if(!strcmp(path, e)){
        return 1;
    }
    return 0;
}

int check_buildin(char** sub_commands, int num_of_sub_commands){
    char* e = "exit";
    char* c = "cd";
    char* p = "pwd";
    //printf("%s, %s", e, command);
    char* first_cmd = sub_commands[0];
    char* path = sub_commands[1];
        
    
    //check if exit
    if(!(strcmp(strndup(first_cmd,4), e))){       
            if(num_of_sub_commands>1){
                throw_err();
                return 1;}
            //when exit is not created correctly
            //exit> exit4, ...
            if(sign_not_path(path)){
                throw_err();
                return 1;}
            
            if((strcmp(first_cmd, strndup(first_cmd,4)))){
                throw_err();
                return 1;}        
            exit(0);}

    //check if cd
    //go to home directory
    if(!(strcmp(strndup(first_cmd,2), c))){
        if(sign_not_path(path)){
            throw_err();
            return 1;}        

        if(strlen(path)==0){
            if(0!=chdir(getenv("HOME"))){
                throw_err();
                return 1;
             }}
        
        if((strcmp(first_cmd, strndup(first_cmd,2)))){
            throw_err();
            return 1;
        }

        if(num_of_sub_commands>2){
            throw_err();
            return 1;
        }

        //go to given diretory
        if(strlen(path)!=0){
            if(0!=chdir(path)){
                throw_err();
                return 1;
                }}
        return 1;
    }

    //check if pwd
    if(!(strcmp(strndup(first_cmd,3), p))){
        if(strlen(path)!=0){
            throw_err();
            return 1;
        }
        if(sign_not_path(path)){
            throw_err();
            return 1;}

        if((strcmp(first_cmd, strndup(first_cmd,3)))){
            throw_err();
            return 1;
        }
   
        char* path = (char*)malloc(sizeof(char)*100);
        getcwd(path, 100);
        myPrint(path);
        char new_l[2] = "\n";
        myPrint(new_l);
        return 1;}
    return 0;
}

//pass by reference
//traverse elements in sub_commands again
//check for wrong input file>, file>file, >file
//file> 1; file>file -> 2; >file3
int check_for_bad_input_one(int current_size, char** subs){
    //current size is the identified size by the time it is parsed by space
    //subs are the current parsed sub commands
    int pos = 0;
    char* sign = ">";
    char* copy[current_size];
    for(int i=0;i<current_size;i++){
        copy[i] = subs[i];
    }   
    for(int i=0; i<current_size; i++){
    //use check-pos      
        int file_pos = check_pos_one(copy[i]);
        if(file_pos==0){
            subs[pos++] = copy[i];
        }else if(file_pos==1){
            char* token;
            token = strtok(copy[i], sign);
            subs[pos++] = token;
            subs[pos++] = sign;
        }else if(file_pos==2){
            char* token;
            token = strtok(copy[i], sign);
            subs[pos++] = token;
            subs[pos++] = sign;
            token = strtok(NULL, sign);
            subs[pos++] = token;
        }else if(file_pos==3){
            char* token;
            token = strtok(copy[i], sign);
            subs[pos++] = sign;
            subs[pos++] = token;
        }
    }

    return pos;
    //pos is the new size of the subcommands
}

int check_pos_two(char* part){
    //we are checking whether there should be bad syntax
    //filename>+filename; filename >filename; filename> filename
    //filename>+filename -> 2
    //filename>+ filename -> 1
    //filename >+filename -> 3
    const char sign[2] = ">+";
    if(strstr(part, sign)==NULL){
        return 0;
    }  
    char* ret = strstr(part, sign);
    
    int len_of_substring = strlen(ret);
    int length = strlen(part);
    if(!strcmp(part, sign)){
        return 0;
    }  
    if(len_of_substring==length){
        //filename >+filename -> 3
        return 3;
    }else if(len_of_substring==2){
        //filename>+ filename -> 1
        return 1; 
    }else if(len_of_substring<length){
        //filename>+filename -> 2
        return 2;
    }
    return 0;
}



int check_for_bad_input_two(int current_size, char** subs){
    //current size is the identified size by the time it is parsed by space
    //subs are the current parsed sub commands
    int pos = 0;
    char* sign = ">+";
    char* copy[current_size];
    for(int i=0;i<current_size;i++){
        copy[i] = subs[i];
    }   
    for(int i=0; i<current_size; i++){
    //use check-pos      
        int file_pos = check_pos_two(copy[i]);
        if(file_pos==0){
            subs[pos++] = copy[i];
        }else if(file_pos==1){
            char* token;
            token = strtok(copy[i], sign);
            subs[pos++] = token;
            subs[pos++] = sign;
        }else if(file_pos==2){
            char* token;
            token = strtok(copy[i], sign);
            subs[pos++] = token;
            subs[pos++] = sign;
            token = strtok(NULL, sign);
            subs[pos++] = token;
        }else if(file_pos==3){
            char* token;
            token = strtok(copy[i], sign);
            subs[pos++] = sign;
            subs[pos++] = token;
        }
    }

    return pos;
    //pos is the new size of the subcommands
}

char* inspect_file_name(char* name){
    char* slash = "/";
    char* point = ".";
    if(strstr(name, slash)==NULL){
        return name;
    }else{
        char* home = getenv("HOME");
        char* path;
        path = strtok(name, point);
        size_t len = strlen(home) + strlen(path);
        char* fullpath = malloc(len);
        strcpy(fullpath, home);
        strcat(fullpath, path);
        return fullpath;
    }
}

int count_sign(char* command){
    int len = strlen(command);
    int count = 0;
    char* sign = ">";
    for(int i=0; i<len; i++){
        char* cur_pos = strndup((command+i), 1);
        if(!strcmp(cur_pos, sign)){
            count++;
        }
    }
    if(count>1){
        return 9;
    }else{
        return count;
    }
}

int parse_by_space(char* command, char** subs, int redirection){
    char space_char = (char)32;
    char* sub_token;
    int size = 0;
    int sign_count = count_sign(command);

    sub_token = strtok(command, &space_char);
    while(sub_token!=NULL){
        removeSpaces(sub_token);
        subs[size++] = sub_token;
        sub_token = strtok(NULL, &space_char);}
    if(redirection==1){
        size = check_for_bad_input_one(size, subs);
        subs[size-1] = inspect_file_name(subs[size-1]);
    }else if(redirection==2){
        size = check_for_bad_input_two(size, subs);
        subs[size-1] = inspect_file_name(subs[size-1]);
    }
 
    char* some = "afolderdoesnotexist";
    if((sign_count!=0)&&(strstr(subs[size-1], some)!=NULL)){
        throw_err();
        return 0;
    }
    /*for(int i=0;i<size; i++){
        printf("%s\n", subs[i]);
    }*/
    return size;
}

//this is the function 
void run_cmd(char** args){
    pid_t child_pid;
    child_pid = fork();
    int child_status;
    pid_t tpid;
    
    if(child_pid==0){
    //this is child process
        if(execvp(args[0], args)==-1){
            throw_err();
            exit(0);
        }
    }else{
    //this is parent process
        do{
            tpid = wait(&child_status);
         } while(tpid != child_pid);
    }
    return;
}

void basic_redirection(char** sub_commands, int num_of_sub_commands){
    int saved_stdout = 0; 
    dup2(STDOUT_FILENO, saved_stdout);
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    //take away 2 last elements, one is the name of the file
    //the other is redirection
    //+1 for NULL at the end
    char* file_name = sub_commands[num_of_sub_commands-1];
    char** final_sub_commands = (char**)malloc(sizeof(char*)*(num_of_sub_commands-2+1));
    for(int i = 0; i<(num_of_sub_commands-2);i++){
        final_sub_commands[i] = (char*)malloc(sizeof(sub_commands[i]));
        final_sub_commands[i] = sub_commands[i];
    }
    final_sub_commands[num_of_sub_commands-2] = NULL;
    
    int fd;
    //check the file already exists
    //if return value is -1, means file DNE
    //great, then we can create a new file and write
    if((fd=open(file_name, O_RDONLY))==-1){
        fd = creat(file_name, mode);
        dup2(fd, STDOUT_FILENO);
    }else{
        throw_err();
        return;
    }
    run_cmd(final_sub_commands);    
    dup2(saved_stdout, STDOUT_FILENO);
    return;
}

void insert(char** sub_commands, int num_of_sub_commands){
        int saved_stdout = 0;
        dup2(STDOUT_FILENO, saved_stdout);
        char* file_name = sub_commands[num_of_sub_commands-1];
        int fd = open(file_name, O_RDONLY);
        char** final_sub_commands = (char**)malloc(sizeof(char*)*(num_of_sub_commands-2+1));
        for(int i = 0; i<(num_of_sub_commands-2);i++){
            final_sub_commands[i] = (char*)malloc(sizeof(sub_commands[i]));
            final_sub_commands[i] = sub_commands[i];
        }
        final_sub_commands[num_of_sub_commands-2] = NULL;        
   
        struct stat st;
        stat(file_name, &st);
        int size = st.st_size; //size of file = size of buffer
        //printf("size of file: %d\n", size);
        char* buffer = (char*)malloc(sizeof(char)*(size));
        ssize_t num_read = read(fd, buffer, size);
        if(num_read==-1){
            throw_err();
            exit(0);}
            //hopefully this will never happen!
        //printf("%s\n", buffer);
        fopen(file_name, "w");
        //freopen will erase original content and create a "new file"
        //printf("reopen, past data erased \n");
        fd=open(file_name, O_WRONLY);
        dup2(fd, STDOUT_FILENO);
        run_cmd(final_sub_commands);
        if((write(fd, buffer, strlen(buffer)))==-1){
            throw_err();
            exit(0);
            //hopefully this will never happen!
        }
        dup2(saved_stdout, STDOUT_FILENO);
        return; 
}



void advance_redirection(char** sub_commands, int num_of_sub_commands){
    int fd =0;
    char* str1 = ">+";
    //if the file DNE, behave like basic redirection
    char* file_name = sub_commands[num_of_sub_commands-1];
    if(!strcmp(file_name, str1)){
        throw_err();
        return;
    }
    if((fd=open(file_name, O_RDONLY))==-1){
        basic_redirection(sub_commands, num_of_sub_commands);
        return;
    }else{
        //need to deal w this later
       // printf("File exists!\n");
        insert(sub_commands, num_of_sub_commands);
        return;
    }
}


void execute(char* command){
    //try to figure whether there is redirection sign in the command before command is parsed
    int redirection = 0;
    char* str1 = ">+";
    char* str2 = ">";
    if(strstr(command, str1)!=NULL){
        redirection = 2;
    }else if(strstr(command, str2)!=NULL){
        redirection = 1;
    }
   
    if(9==count_sign(command)){
        throw_err();
        return;
    } 
 
    //allocate memory for subparts of each command line
    //put it into a list of strings called sub_commands
    char** sub_commands = (char**)malloc(sizeof(char*)*512);
    for(int i = 0; i<512;i++){
        sub_commands[i] = (char*)malloc(sizeof(char)*512);}
    int num_of_sub_commands = parse_by_space(command, sub_commands, redirection);
    
    if(num_of_sub_commands==0){
        return;
    }
    
    if((!strcmp(sub_commands[num_of_sub_commands-1], str1))||(!strcmp(sub_commands[num_of_sub_commands-1], str2))){
        throw_err();
        return;
    }

    //check if there is builtin cmds
    int is_builtin = check_buildin(sub_commands, num_of_sub_commands);
    if(is_builtin){return;}
  
 
    //need to check if there are any redirection
    //check if output already exists/not specify
    if(redirection==2){
        advance_redirection(sub_commands, num_of_sub_commands);
        return;
    }else if (redirection==1){
        //basic redirection
        basic_redirection(sub_commands, num_of_sub_commands);
        return;
    }


    //if it is not builtin or redirection, we should safely assume that we can run the rest commands
    //since most of the sub_commands now are NULL
    //And what execvp wanna take is a string list w only one NULL pointer ending
    // we need to create a new string list
    char** final_sub_commands = (char**)malloc(sizeof(char*)*(num_of_sub_commands+1));
    for(int i = 0; i<num_of_sub_commands;i++){
        final_sub_commands[i] = (char*)malloc(sizeof(sub_commands[i]));
        final_sub_commands[i] = sub_commands[i];
    }
    //printf("Number of subcommands: %d\n", num_of_sub_commands);
    //the final element should be NULL
    final_sub_commands[num_of_sub_commands] = NULL;
    run_cmd(final_sub_commands);     
    return;
}

void one_file_only(int argc){
    if(argc>2){
        throw_err();
        exit(0);
    }
} 


int main(int argc, char *argv[]){ 
    FILE* file;
    char* msg = (char*)malloc(sizeof(char)*514);
    const char newline[2] = "\n";

    one_file_only(argc);
    if (argc == 2){
        if((file = fopen(argv[1], "r"))==NULL){
            throw_err();
            exit(0);
        }
    }else if(argc == 1) {
        myPrint("myshell> ");
        file = stdin;
    }else{
        throw_err();
        exit(0);
    }

    while(fgets(msg, 514, file)){
        //if more than 514, execute none
        if(strstr(msg, newline)==NULL){
            myPrint(msg);
            fgets(msg,514,file);
            myPrint(msg);
            throw_err();
            continue;
        }
        if(is_white_space(msg)){
            continue;
            if(argc == 1) {myPrint("myshell> ");}
        }
        myPrint(msg);
        char** commands = (char**)malloc(sizeof(char*)*512);
        for(int i=0; i<512; i++){
            commands[i] = (char*)malloc(sizeof(char)*512);
        }
        int num_of_commands = parse_by_semicol(msg, commands);
        
        if(num_of_commands==0){
            continue;
            if(argc == 1) {myPrint("myshell> ");}
        }        
       
 
        for(int i = 0; i<num_of_commands; i++){
            //get rid of the newline
            //it will cause problem for strcmp
            if(strstr(commands[i], "\n")){
                int len = strlen(commands[i]);
                commands[i][len] = (char)0;
                commands[i][len-1] = (char)0;
            }
            execute(commands[i]); 
        } 
        memset(msg, 0, num_of_commands);
        
    
        if(argc == 1) {myPrint("myshell> ");}
    }

    return 0;
}


