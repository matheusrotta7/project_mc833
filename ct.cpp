#include <bits/stdc++.h>

using namespace std;

int main() {


    FILE* res_server;
    FILE* res_client;

    res_server = fopen("results_exp6_server.txt", "r");
    res_client = fopen("results_exp6_client.txt", "r");

    for (int i = 0; i < 20; i++) {
        int dummy, time_on_client, time_on_server;
        fscanf(res_client, "ITERATION %d -- TOTAL TIME ON CLIENT : %d\n", &dummy, &time_on_client);
        fscanf(res_server, "ITERATION %d -- TOTAL TIME ON SERVER : %d\n", &dummy, &time_on_server);
        printf("ITERATION %d -- COMMUNICATION TIME: %d\n", i+1, time_on_client - time_on_server);
    }

    fclose(res_server);
    fclose(res_client);


    return 0;
}
