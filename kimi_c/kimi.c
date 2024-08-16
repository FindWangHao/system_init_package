#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include <curl/curl.h>

#define URL "https://api.moonshot.cn/v1/chat/completions"
#define API_KEY "sk-tM7KvOhZaP3uW1pdtbmVUfmOQBwOQQVdZcxa0svefcOE2pVt"

struct string {
  char *ptr;
  size_t len;
};

void init_string(struct string *s) {
  s->len = 0;
  s->ptr = malloc(s->len + 1);
  if (s->ptr == NULL) {
    fprintf(stderr, "malloc() failed\n");
    exit(EXIT_FAILURE);
  }
  s->ptr[0] = '\0';
}

size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s) {
  size_t new_len = s->len + size * nmemb;
  s->ptr = realloc(s->ptr, new_len + 1);
  if (s->ptr == NULL) {
    fprintf(stderr, "realloc() failed\n");
    exit(EXIT_FAILURE);
  }
  memcpy(s->ptr + s->len, ptr, size * nmemb);
  s->ptr[new_len] = '\0';
  s->len = new_len;

  return size * nmemb;
}

// 定义结构体来存储解析后的数据
typedef struct {
    char id[50];
    char object[50];
    long created;
    char model[50];
    int index;
    char role[50];
    char *content;  // 动态分配内存
    char finish_reason[50];
    int prompt_tokens;
    int completion_tokens;
    int total_tokens;
} ChatCompletion;

// 解析JSON字符串并存储到结构体中
void parse_json(ChatCompletion* chatCompletion, const char *json_str) {
    // 解析JSON字符串
    cJSON *json = cJSON_Parse(json_str);
    if (json == NULL) {
        printf("Error parsing JSON\n");
        exit(1);
    }
    // 提取并存储字段值
    cJSON *id = cJSON_GetObjectItemCaseSensitive(json, "id");
    cJSON *object = cJSON_GetObjectItemCaseSensitive(json, "object");
    cJSON *created = cJSON_GetObjectItemCaseSensitive(json, "created");
    cJSON *model = cJSON_GetObjectItemCaseSensitive(json, "model");
    cJSON *choices = cJSON_GetObjectItemCaseSensitive(json, "choices");
    cJSON *usage = cJSON_GetObjectItemCaseSensitive(json, "usage");
    // 提取choices数组中的内容
    cJSON *choice = cJSON_GetArrayItem(choices, 0);
    cJSON *message = cJSON_GetObjectItemCaseSensitive(choice, "message");
    cJSON *index = cJSON_GetObjectItemCaseSensitive(choice, "index");
    cJSON *role = cJSON_GetObjectItemCaseSensitive(message, "role");
    cJSON *content = cJSON_GetObjectItemCaseSensitive(message, "content");
    cJSON *finish_reason = cJSON_GetObjectItemCaseSensitive(choice, "finish_reason");
    // 提取usage中的内容
    cJSON *prompt_tokens = cJSON_GetObjectItemCaseSensitive(usage, "prompt_tokens");
    cJSON *completion_tokens = cJSON_GetObjectItemCaseSensitive(usage, "completion_tokens");
    cJSON *total_tokens = cJSON_GetObjectItemCaseSensitive(usage, "total_tokens");
    // 复制字段值到结构体
    strcpy(chatCompletion->id, id->valuestring);
    strcpy(chatCompletion->object, object->valuestring);
    chatCompletion->created = created->valueint;
    strcpy(chatCompletion->model, model->valuestring);
    chatCompletion->index = index->valueint;
    strcpy(chatCompletion->role, role->valuestring);

    // 动态分配内存并复制内容
    chatCompletion->content = malloc(strlen(content->valuestring) + 1);
    if (chatCompletion->content == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    strcpy(chatCompletion->content, content->valuestring);

    strcpy(chatCompletion->finish_reason, finish_reason->valuestring);
    chatCompletion->prompt_tokens = prompt_tokens->valueint;
    chatCompletion->completion_tokens = completion_tokens->valueint;
    chatCompletion->total_tokens = total_tokens->valueint;

    // 释放JSON对象
    cJSON_Delete(json);
}

void content_print(ChatCompletion* chatCompletion){
    printf("本次回答一共耗费tokens：%d\n", chatCompletion->total_tokens);
    printf("答: %s\n", chatCompletion->content);
}

typedef struct {
    char *role;
    char *content;
} Message;

typedef struct {
    Message *messages;
    size_t size;
} MessageHistory;

void init_message_history(MessageHistory *history) {
    history->messages = NULL;
    history->size = 0;
}

void add_message(MessageHistory *history, const char *role, const char *content) {
    history->messages = realloc(history->messages, (history->size + 1) * sizeof(Message));
    if (history->messages == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    history->messages[history->size].role = strdup(role);
    history->messages[history->size].content = strdup(content);
    if (history->messages[history->size].role == NULL || history->messages[history->size].content == NULL) {
        fprintf(stderr, "strdup() failed\n");
        exit(EXIT_FAILURE);
    }
    history->size++;
}

void free_message_history(MessageHistory *history) {
    for (size_t i = 0; i < history->size; i++) {
        free(history->messages[i].role);
        free(history->messages[i].content);
    }
    free(history->messages);
}

char* create_json_data(MessageHistory *history) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "model", "moonshot-v1-8k");
    cJSON *messages = cJSON_AddArrayToObject(root, "messages");

    for (size_t i = 0; i < history->size; i++) {
        cJSON *message = cJSON_CreateObject();
        cJSON_AddStringToObject(message, "role", history->messages[i].role);
        cJSON_AddStringToObject(message, "content", history->messages[i].content);
        cJSON_AddItemToArray(messages, message);
    }

    cJSON_AddNumberToObject(root, "temperature", 0.3);
    char *json_data = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return json_data;
}

int main(void) {
    MessageHistory history;
    init_message_history(&history);

    // Add initial system message
    add_message(&history, "system", "你是 Kimi，由 Moonshot AI 提供的人工智能助手。我和你的对话可能是英文、中文或拼音。");

    ChatCompletion chatCompletion;
    chatCompletion.content = NULL;
    CURL *curl;
    CURLcode res;

    struct string s;
    init_string(&s);

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        char auth_header[256];
        snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", API_KEY);
        headers = curl_slist_append(headers, auth_header);

        curl_easy_setopt(curl, CURLOPT_URL, URL);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        while (1) {
            char user_input[256];
            printf("请输入你的问题 (输入 'exit' 退出): ");
            fgets(user_input, sizeof(user_input), stdin);

            size_t len = strlen(user_input);
            if (len > 0 && user_input[len - 1] == '\n') {
                user_input[len - 1] = '\0';
            }

            if (strcmp(user_input, "exit") == 0) {
                break;
            }

            add_message(&history, "user", user_input);

            char *json_data = create_json_data(&history);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);

            res = curl_easy_perform(curl);

            if (res != CURLE_OK) {
                fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            } else {
                parse_json(&chatCompletion, s.ptr);
                content_print(&chatCompletion);
                add_message(&history, "assistant", chatCompletion.content);
            }

            free(json_data);
            free(s.ptr);
            init_string(&s);
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    free(s.ptr);
    if (chatCompletion.content != NULL) {
        free(chatCompletion.content);
    }
    free_message_history(&history);
    curl_global_cleanup();

    return 0;
}

