#define _GNU_SOURCE
#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#define PENGLAI_ENCLAVE_DEV_PATH "/dev/penglai_linux_dev"

#define PENGLAI_IOC_ENTER_SECURE 1

int fd;

void sbi_domain_secure_enter()
{
    int ret = ioctl(fd, PENGLAI_IOC_ENTER_SECURE, NULL);
    if (ret < 0)
    {
        fprintf(stderr, "LIB: ioctl enter secure is failed\n");
    }
}

void *print_hello_cpu0(void *arg)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    while (1)
    {
        printf("Hello CPU0\n");
        sbi_domain_secure_enter();
        printf("END CPU0\n");
        sleep(3);
    }
    return NULL;
}

void *print_hello_cpu1(void *arg)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);
    pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    while (1)
    {
        printf("Hello CPU1\n");
        sbi_domain_secure_enter();
        printf("END CPU1\n");
        sleep(3);
    }
    return NULL;
}

int main()
{
    pthread_t thread_cpu0, thread_cpu1;

    fd = open(PENGLAI_ENCLAVE_DEV_PATH, O_RDWR);
    if (fd < 0)
    {
        fprintf(stderr, "LIB: cannot open enclave dev\n");
    }

    pthread_create(&thread_cpu0, NULL, print_hello_cpu0, NULL);
    pthread_create(&thread_cpu1, NULL, print_hello_cpu1, NULL);

    pthread_join(thread_cpu0, NULL);
    pthread_join(thread_cpu1, NULL);

    return 0;
}
