Import("env")

env.Replace(PROGNAME="firmware-cosmo-ha-mqtt-adapter-%s" % env.GetProjectOption("firm_version"))