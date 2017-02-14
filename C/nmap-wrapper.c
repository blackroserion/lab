/***************************
 * rion@c0d3r
 * here, take my dirty code
 ***************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LEN 256
#define MAX_CMD 1024
#define MAX_OUT 1024*1024


void print_state(char *buff, int status){
    if(status)
        fprintf(stdout, "%s", buff);
}

void error(char *str){
    fprintf(stderr, "%s\n", str);
    exit(1);
}


void help(void){
    fprintf(stdout, "Usage: nmap-wrapper <host> <mode>\n"
                    "List Modes:\n"
                    "0 : Scan single host\n"
                    "1 : Turn on OS and version detection scanning\n"
                    "2 : Find out if a host/network is protected by a firewall\n"
                    "3 : Scan a host when protected by the firewall\n"
                    "4 : Perform fast scanning\n"
                    "5 : Display the reason a port in a particular state\n"
                    "6 : Only show open (or possibly open) ports\n"
                    "7 : Show all packets sent and received\n"
                    "8 : Detect remote operating system\n"
                    "9 : Detect remote services (server/daemon) version numbers\n"
                    "10: Scan a host using TCP ACK and TCP Syn ping\n"
                    "11: Scan a host using IP protocol ping\n"
                    "12: Scan a host using UDP ping\n"
                    "13: Scan a host for UDP services (UDP Scan)\n"
                    "14: Scan for IP protocol\n"
                    "15: Scan a firewall for packets fragments\n"
                    "16: Scan a firewall for MAC address spoofing\n"
                    "90: Intense scan plus UDP\n"
                    "91: Intense scan, all TCP ports\n"
                    "92: Intense scan, no ping\n"
                    "93: Slow comprehensive scan\n");
    exit(0);
}

int main(int argc, char *argv[]){
    if(argc < 3){
        if(argc < 2)
            error("Usage: nmap-wrapper <host> <mode>\n"
                "Or see -h for list modes");
        else if(argc == 2 && strncmp(argv[1], "-h", 2))
            error("Usage: nmap-wrapper <host> <mode>\n"
                "Or see -h for list modes");
        else
            help();
    }

    int mode = atoi(argv[2]);
    if(!mode && mode != 0)
        error("Mode must be integer");
    char *host = argv[1];
    char cmd[MAX_CMD];

    strcpy(cmd, "nmap ");
    switch(mode)
    {
        case 0: strcat(cmd, "-v ");break;
        case 1: strcat(cmd, "-A -v ");break;
        case 2: strcat(cmd, "-sA ");break;
        case 3: strcat(cmd, "-PN ");break;
        case 4: strcat(cmd, "-T4 -F ");break;
        case 5: strcat(cmd, "--reason ");break;
        case 6: strcat(cmd, "--open ");break;
        case 7: strcat(cmd, "--packet-trace ");break;
        case 8: strcat(cmd, "-v -O --osscan-guess ");break;
        case 9: strcat(cmd, "-sV ");break;
        case 10: strcat(cmd, "-PS -PA ");break;
        case 11: strcat(cmd, "-PO ");break;
        case 12: strcat(cmd, "-PU ");break;
        case 13: strcat(cmd, "-sU ");break;
        case 14: strcat(cmd, "-sO ");break;
        case 15: strcat(cmd, "-f ");break;
        case 16: strcat(cmd, "--spoof-mac ");break;
        case 90: strcat(cmd, "-sS -sU -T4 -A -v ");break;
        case 91: strcat(cmd, "-p 1-65535 -T4 -A -v ");break;
        case 92: strcat(cmd, "-T4 -A -v -Pn ");break;
        case 93: strcat(cmd, "-sS -sU -T4 -A -v -PE -PP "
                             "-PS80,443 -PA3389 "
                             "-PU40125 -PY -g53 --script "
                             "'default or (discovery and safe)' ");break;
        default: error("Unknown Mode!");break;
    }
    strcat(cmd, host);

    FILE *fp = popen(cmd, "r");
    char buff[MAX_OUT];
    int flag = 0;
    if(fp)
    {
        while(fgets(buff, MAX_LEN, (FILE*)fp))
        {
            if(!strncmp(buff, "PORT", 4)        ||
                !strncmp(buff, "PROTOCOL", 8)   ||
                !strncmp(buff, "SENT", 4)       ||
                !strncmp(buff, "CONN", 4)       ||
                !strncmp(buff, "FAILED", 6)
              ) flag = 1;
            else if( !strncmp(buff, "Read", 4)  ||
                !strncmp(buff, "Nmap", 4)       ||
                !strncmp(buff, "Completed", 9)  ||
                !strncmp(buff, "NSE", 3)        ||
                !strncmp(buff, "Service", 7)
              ) flag = 0;
            print_state(buff, flag);
        }
        fclose(fp);
    }
    else{
        fputs("Error while opening nmap", stdout);
    }

    return 0;
}
