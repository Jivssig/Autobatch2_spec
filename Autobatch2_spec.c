#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_FILES 1000
#define MAX_FILENAME 256

typedef struct {
    char name[MAX_FILENAME];
    int selected;
    int number;
} FileInfo;

int extract_number_from_filename(const char *filename) {
    int num = 0;
    int found_digit = 0;
    
    for (int i = 0; filename[i] != '\0'; i++) {
        if (filename[i] >= '0' && filename[i] <= '9') {
            num = num * 10 + (filename[i] - '0');
            found_digit = 1;
        } else if (found_digit) {
            break;
        }
    }
    return num;
}

// 检查是否为txt文件
int is_txt_file(const char *filename) {
    int len = strlen(filename);
    if (len < 4) return 0;
    return (strcmp(filename + len - 4, ".txt") == 0);
}

// 比较函数用于排序
int compare_files(const void *a, const void *b) {
    const FileInfo *fa = (const FileInfo *)a;
    const FileInfo *fb = (const FileInfo *)b;
    return fa->number - fb->number;
}

int main() {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    FileInfo files[MAX_FILES];
    int file_count = 0;
    int i;
    
    printf("正在扫描当前目录下的txt文件...\n");
    
    // 1. 扫描当前目录下的txt文件
    dir = opendir(".");
    if (dir == NULL) {
        perror("无法打开当前目录");
        return EXIT_FAILURE;
    }
    
    while ((entry = readdir(dir)) != NULL && file_count < MAX_FILES) {
        if (is_txt_file(entry->d_name)) {
            strncpy(files[file_count].name, entry->d_name, MAX_FILENAME - 1);
            files[file_count].name[MAX_FILENAME - 1] = '\0';
            files[file_count].selected = 0;
            files[file_count].number = extract_number_from_filename(entry->d_name);
            file_count++;
        }
    }
    closedir(dir);
    
    if (file_count == 0) {
        printf("当前目录下没有找到txt文件！\n");
        return EXIT_FAILURE;
    }
    
    // 2. 按数字排序
    qsort(files, file_count, sizeof(FileInfo), compare_files);
    
    // 3. 显示文件列表
    printf("\n找到 %d 个txt文件:\n", file_count);
    printf("序号\t文件状态\t文件名\n");
    printf("--------------------------------------------------\n");
    
    for (i = 0; i < file_count; i++) {
        printf("%3d.\t[%c]\t\t%s (数字: %d)\n", 
               i + 1, 
               files[i].selected ? 'X' : ' ',
               files[i].name,
               files[i].number);
    }
    
    // 4. 用户选择文件
    printf("\n请选择要转换的文件:\n");
    printf("1. 输入文件序号（如: 1 3 5）\n");
    printf("2. 输入范围（如: 1-10）\n");
    printf("3. 输入 '*' 选择所有文件\n");
    printf("4. 输入 'q' 退出程序\n");
    printf("\n请选择: ");
    
    char choice[100];
    fgets(choice, sizeof(choice), stdin);
    
    // 去除换行符
    choice[strcspn(choice, "\n")] = 0;
    
    if (choice[0] == 'q' || choice[0] == 'Q') {
        printf("程序退出。\n");
        return EXIT_SUCCESS;
    }
    
    // 处理用户选择
    if (choice[0] == '*') {
        // 选择所有文件
        for (i = 0; i < file_count; i++) {
            files[i].selected = 1;
        }
        printf("已选择所有 %d 个文件。\n", file_count);
    } else if (strchr(choice, '-') != NULL) {
        // 处理范围选择
        int start, end;
        if (sscanf(choice, "%d-%d", &start, &end) == 2) {
            if (start > 0 && end <= file_count && start <= end) {
                for (i = start - 1; i < end; i++) {
                    files[i].selected = 1;
                }
                printf("已选择文件 %d 到 %d。\n", start, end);
            } else {
                printf("无效的范围！\n");
                return EXIT_FAILURE;
            }
        }
    } else {
        // 处理多个序号选择
        char *token = strtok(choice, " ");
        while (token != NULL) {
            int index = atoi(token);
            if (index > 0 && index <= file_count) {
                files[index - 1].selected = 1;
            } else {
                printf("警告: 忽略无效序号 %d\n", index);
            }
            token = strtok(NULL, " ");
        }
    }
    
    // 5. 统计选择的数量
    int selected_count = 0;
    for (i = 0; i < file_count; i++) {
        if (files[i].selected) {
            selected_count++;
        }
    }
    
    if (selected_count == 0) {
        printf("没有选择任何文件！\n");
        return EXIT_SUCCESS;
    }
    
    printf("\n准备转换 %d 个文件:\n", selected_count);
    for (i = 0; i < file_count; i++) {
        if (files[i].selected) {
            printf("  %s\n", files[i].name);
        }
    }
    
    // 6. 确认转换
    printf("\n是否开始转换？(y/n): ");
    char confirm;
    scanf(" %c", &confirm);
    
    if (confirm != 'y' && confirm != 'Y') {
        printf("转换取消。\n");
        return EXIT_SUCCESS;
    }
    
    // 7. 批量转换文件
    int converted_count = 0;
    printf("\n开始转换...\n");
    
    for (i = 0; i < file_count; i++) {
        if (!files[i].selected) {
            continue;
        }
        
        // 构造输入文件名
        char input_file[MAX_FILENAME];
        strcpy(input_file, files[i].name);
        
        // 提取基础文件名（去掉.txt）
        char base_name[MAX_FILENAME];
        strcpy(base_name, input_file);
        char *dot = strrchr(base_name, '.');
        if (dot != NULL) {
            *dot = '\0';
        }
        
        // 构造命令
        char command_str[500];
        snprintf(command_str, sizeof(command_str),
                 "echo -e '6\\nn\\n%s\\n' | your_path_to/spec_conv",    ///////your path
                 input_file);
        
        // 执行转换命令
        printf("[%d/%d] 转换: %s -> %s_8k.spec\n", 
               converted_count + 1, selected_count,
               input_file, base_name);
        
        int result = system(command_str);
        
        // 错误处理
        if (result != 0) {
            fprintf(stderr, "错误: %s 转换失败 (代码 %d)\n",
                    input_file, result);
        } else {
            converted_count++;
        }
    }
    
    printf("\n批量转换完成! 成功转换 %d/%d 个文件\n", 
           converted_count, selected_count);
    
    if (converted_count < selected_count) {
        printf("有 %d 个文件转换失败。\n", selected_count - converted_count);
    }
    
    return EXIT_SUCCESS;
}
