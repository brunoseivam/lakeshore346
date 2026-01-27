#include <dbAccessDefs.h>
#include <errlog.h>
#include <iocsh.h>
#include <registryFunction.h>

#include <asynPortClient.h>
#include <asynDriver.h>
#include <asynOctet.h>

#include "lakeshore346_util.h"

static void lakeshore346_autoload_db(const char *db_path, const char *port, int addr, const char *macros) {
    asynOctetClient client = asynOctetClient(port, addr, NULL);

    const char *SLOT_NAMES = "EFGH";

    // Map card type to number of inputs
    static const struct {
        const char *name;
        size_t num_inputs;
    } CARD_DATA[] {
        /* 0 */{ "Empty", 0 },
        /* 1 */{ "Scanner", 4 },
        /* 2 */{ "Unpublished", 0 },
        /* 3 */{ "Unpublished", 0 },
        /* 4 */{ "Thermocouple", 2 },
    };

    int slots[4] = {0,0,0,0};
    try {
        // Assume response is of the format %d,%d,%d,%d
        std::string response = read_write(client, "CARDS?");
        sscanf(response.c_str(), "%d,%d,%d,%d", &slots[0], &slots[1], &slots[2], &slots[3]);
    } catch (const std::exception & ex) {
        errlogSevPrintf( errlogMajor, "%s: failed to read installed cards: %s\n", __FUNCTION__, ex.what());
        errlogSevPrintf( errlogMajor, "%s: loading records for ALL potential inputs\n", __FUNCTION__);
        slots[0] = slots[1] = slots[2] = slots[3] = 1; // Pretend there's a scanner card in all slots
    }

    // Load core records
    {
        char filename[4096];
        snprintf(filename, sizeof(filename), "%s/lakeshore346_core.db", db_path);
        printf("Loading core records\n");
        printf("dbLoadRecords(\"%s\",\"%s\")\n", filename, macros);
        dbLoadRecords(filename, macros);
    }
    // Load per-input records
    for (size_t slot = 0; slot < sizeof(slots)/sizeof(slots[0]); ++slot) {
        char slot_name = SLOT_NAMES[slot];
        int card_type = slots[slot];
        const char *card_name = CARD_DATA[card_type].name;
        size_t num_inputs = CARD_DATA[card_type].num_inputs;

        char filename[4096];
        snprintf(filename, sizeof(filename), "%s/lakeshore346_input.template", db_path);

        for (size_t i = 0; i < num_inputs; ++i) {
            char substitutions[4096];
            snprintf(
                substitutions, sizeof(substitutions),
                "INPUT=%c%lu,CARD=%c,CARDINP=%lu,%s",
                slot_name, i+1, slot_name, i, macros
            );
            printf("Loading records for card %c (%s) input %c%lu\n", slot_name, card_name, slot_name, i+1);
            printf("dbLoadRecords(\"%s\",\"%s\")\n", filename, substitutions);
            dbLoadRecords(filename, substitutions);
        }
    }
}

// EPICS registration code
static const iocshArg lakeshore346_autoload_db_arg0 = { "DB Path", iocshArgStringPath };
static const iocshArg lakeshore346_autoload_db_arg1 = { "Port",    iocshArgString     };
static const iocshArg lakeshore346_autoload_db_arg2 = { "Addr",    iocshArgInt        };
static const iocshArg lakeshore346_autoload_db_arg3 = { "Macros",  iocshArgString     };
static const iocshArg * const lakeshore346_autoload_db_args[4] = {
    &lakeshore346_autoload_db_arg0, &lakeshore346_autoload_db_arg1,
    &lakeshore346_autoload_db_arg2, &lakeshore346_autoload_db_arg3
};
static const iocshFuncDef lakeshore346_autoload_db_func_def = {
    "lakeshore346_autoload_db", 4, lakeshore346_autoload_db_args
};
static void lakeshore346_autoload_db_call_func(const iocshArgBuf *args) {
    lakeshore346_autoload_db(args[0].sval, args[1].sval, args[2].ival, args[3].sval);
}
static void lakeshore346_registrar(void) {
    iocshRegister(&lakeshore346_autoload_db_func_def, lakeshore346_autoload_db_call_func);
}

#include <epicsExport.h>
epicsExportRegistrar(lakeshore346_registrar);
