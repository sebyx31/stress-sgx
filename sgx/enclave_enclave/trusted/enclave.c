#include "enclave_t.h"  /* print_string */
#include "stress-cpu.c"
#include <stdio.h>

/*
 *  keep_stressing()
 *	returns true if we can keep on running a stressor
 */
bool HOT OPTIMIZE3 keep_stressing(const uint64_t rounds, uint64_t *const counter)
{
	return (LIKELY(*g_keep_stressing_flag) &&
				LIKELY(!rounds || ((*counter) < rounds)));
}

void run_stressor(const stress_cpu_method_info_t* info, const uint64_t rounds, uint64_t *const counter) {
	do {
		(info->func)("stress-sgx");
		(*counter)++;
	} while(keep_stressing(rounds, counter));
}

int ecall_method_exists(const char* method_name)
{
	stress_cpu_method_info_t const *info;
	for (info = cpu_methods; info->func; info++) {
		if (strcmp(info->name, method_name) == 0) {
			return 1;
		}
	}
	return 0;
}

void ecall_get_methods_error(char* out_methods, int length)
{
	stress_cpu_method_info_t const *info;
	int counter = 0;

	if (sgx_is_outside_enclave(out_methods, length)) {
		counter += snprintf(out_methods, length, "sgx-method must be one of:");
		for (info = cpu_methods; info->func; info++) {
			counter += snprintf(out_methods + counter, length - counter, " %s", info->name);
		}
		(void)snprintf(out_methods + counter, length - counter, "\n");
	}
}

int ecall_stress_cpu(const char* method_name, const uint64_t rounds, uint64_t *const counter , bool* keep_stressing_flag, uint64_t opt_flags)
{
	stress_cpu_method_info_t const *info;

	g_opt_flags = opt_flags;
	g_keep_stressing_flag = keep_stressing_flag;

	if (g_keep_stressing_flag == 0) {
		return -1;
	}

	for (info = cpu_methods; info->func; info++) {
		if (!strcmp(info->name, method_name)) {
			run_stressor(info, rounds, counter);
			return 0;
		}
	}

	return -1;
}
