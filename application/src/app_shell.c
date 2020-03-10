#include <shell/shell.h>

LOG_MODULE_REGISTER(app_shell, LOG_LEVEL_DBG);

static int cmd_app_dummy(const struct shell *shell, size_t argc, char **argv)
{
	LOG_DBG("cmd_app_dummy");

	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
    sub_app,
    SHELL_CMD(dummy, NULL, "APP", cmd_app_dummy),
    SHELL_SUBCMD_SET_END);

SHELL_CMD_REGISTER(app, &sub_app, "APP", NULL);
