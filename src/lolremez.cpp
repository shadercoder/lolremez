//
//  LolRemez — Remez algorithm implementation
//
//  Copyright © 2005—2017 Sam Hocevar <sam@hocevar.net>
//
//  This program is free software. It comes without any warranty, to
//  the extent permitted by applicable law. You can redistribute it
//  and/or modify it under the terms of the Do What the Fuck You Want
//  to Public License, Version 2, as published by the WTFPL Task Force.
//  See http://www.wtfpl.net/ for more details.
//

#if HAVE_CONFIG_H
#   include "config.h"
#endif

#include <lol/engine.h>

#include <lol/math/real.h>

#include "solver.h"
#include "expression.h"

using lol::array;
using lol::real;
using lol::String;

static void version(void)
{
    printf("lolremez %s\n", PACKAGE_VERSION);
    printf("Copyright © 2005—2017 Sam Hocevar <sam@hocevar.net>\n");
    printf("This program is free software. It comes without any warranty, to the extent\n");
    printf("permitted by applicable law. You can redistribute it and/or modify it under\n");
    printf("the terms of the Do What the Fuck You Want to Public License, Version 2, as\n");
    printf("published by the WTFPL Task Force. See http://www.wtfpl.net/ for more details.\n");
    printf("\n");
    printf("Written by Sam Hocevar. Report bugs to <sam@hocevar.net>.\n");
}

static void usage()
{
    printf("Usage: lolremez [-d degree] [-r xmin:xmax] x-expression [x-error]\n");
    printf("       lolremez -h | --help\n");
    printf("       lolremez -V | --version\n"); 
    printf("Find a polynomial approximation for x-expression.\n");
    printf("\n");
    printf("Mandatory arguments to long options are mandatory for short options too.\n");
    printf("  -d, --degree <degree>      degree of final polynomial\n");
    printf("  -r, --range <xmin>:<xmax>  range over which to approximate\n");
    printf("      --progress             print progress\n");
    printf("      --stats                print timing statistics\n");
    printf("  -h, --help                 display this help and exit\n");
    printf("  -V, --version              output version information and exit\n");
    printf("\n");
    printf("Examples:\n");
    printf("  lolremez -d 4 -r -1:1 \"atan(exp(1+x))\"\n");
    printf("  lolremez -d 4 -r -1:1 \"atan(exp(1+x))\" \"exp(1+x)\"\n");
    printf("\n");
    printf("Written by Sam Hocevar. Report bugs to <sam@hocevar.net>.\n");
}

static void FAIL(char const *message = nullptr)
{
    if (message)
        printf("Error: %s\n", message);
    printf("Try 'lolremez --help' for more information.\n");
    exit(EXIT_FAILURE);
}

/* See the tutorial at http://lolengine.net/wiki/doc/maths/remez */
int main(int argc, char **argv)
{
    bool show_stats = false;
    bool show_progress = false;

    remez_solver solver;

    lol::getopt opt(argc, argv);
    opt.add_opt('h', "help",     false);
    opt.add_opt('v', "version",  false);
    opt.add_opt('d', "degree",   true);
    opt.add_opt('r', "range",    true);
    opt.add_opt(200, "stats",    false);
    opt.add_opt(201, "progress", false);

    for (;;)
    {
        int c = opt.parse();
        if (c == -1)
            break;

        switch (c)
        {
        case 'd': /* --degree */
            solver.set_order(atoi(opt.arg));
            break;
        case 'r': { /* --range */
            array<String> arg = String(opt.arg).split(':');
            if (arg.count() != 2)
                FAIL("invalid range");
            expression ex;
            ex.parse(arg[0].C());
            if (!ex.is_constant())
                FAIL("invalid range: xmin must be constant");
            real xmin = ex.eval(real::R_0());
            ex.parse(arg[1].C());
            if (!ex.is_constant())
                FAIL("invalid range: xmin must be constant");
            real xmax = ex.eval(real::R_0());
            if (xmin >= xmax)
                FAIL("invalid range: xmin >= xmax");
            solver.set_range(xmin, xmax);
          } break;
        case 200: /* --stats */
            show_stats = true;
            break;
        case 201: /* --progress */
            show_progress = true;
            break;
        case 'h': /* --help */
            usage();
            return EXIT_SUCCESS;
        case 'v': /* --version */
            version();
            return EXIT_SUCCESS;
        default:
            FAIL();
        }
    }

    if (opt.index >= argc)
        FAIL("no function specified");

    solver.set_func(argv[opt.index++]);

    if (opt.index < argc)
        solver.set_weight(argv[opt.index++]);

    if (opt.index < argc)
        FAIL("too many arguments");

    solver.show_stats = show_stats;

    for (solver.do_init(); solver.do_step(); )
    {
        if (show_progress)
            solver.do_print(remez_solver::format::gnuplot);
        fflush(stdout);
    }

    solver.do_print(remez_solver::format::cpp);

    return 0;
}

