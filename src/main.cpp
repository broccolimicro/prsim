/*
 * main.cpp
 *
 *  Created on: Jan 16, 2015
 *      Author: nbingham
 */

#include <common/standard.h>
#include <parse/parse.h>
#include <parse/default/block_comment.h>
#include <parse/default/line_comment.h>
#include <parse_expression/expression.h>
#include <parse_expression/assignment.h>
#include <parse_expression/composition.h>
#include <parse_prs/production_rule_set.h>
#include <prs/production_rule.h>
#include <prs/simulator.h>
#include <interpret_prs/import.h>
#include <interpret_prs/export.h>
#include <interpret_boolean/export.h>
#include <interpret_boolean/import.h>
#include <ucs/variable.h>

void print_help()
{
	printf("Usage: prsim [options] file...\n");
	printf("A simulation environment for PRS processes.\n");
	printf("\nSupported file formats:\n");
	printf(" *.prs           Load an PRS\n");
	printf(" *.sim           Load a sequence of transitions to operate on\n");
	printf("\nGeneral Options:\n");
	printf(" -h,--help      Display this information\n");
	printf("    --version   Display version information\n");
	printf(" -v,--verbose   Display verbose messages\n");
	printf(" -d,--debug     Display internal debugging messages\n");
	printf("\nConversion Options:\n");
}

void print_version()
{
	printf("prsim 1.0.0\n");
	printf("Copyright (C) 2013 Sol Union.\n");
	printf("There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
	printf("\n");
}

void print_command_help()
{
	printf("<arg> specifies a required argument\n(arg=value) specifies an optional argument with a default value\n");
	printf("\nGeneral:\n");
	printf(" help, h             print this message\n");
	printf(" seed <n>            set the random seed for the simulation\n");
	printf(" source <file>       source and execute a list of commands from a file\n");
	printf(" save <file>         save the sequence of fired transitions to a '.sim' file\n");
	printf(" load <file>         load a sequence of transitions from a '.sim' file\n");
	printf(" clear, c            clear any stored sequence and return to random stepping\n");
	printf(" quit, q             exit the interactive simulation environment\n");
	printf("\nRunning Simulation:\n");
	printf(" tokens, t           list the location and state information of every token\n");
	printf(" enabled, e          return the list of enabled transitions\n");
	printf(" fire <i>, f<i>      fire the i'th enabled transition\n");
	printf(" step (N=1), s(N=1)  step through N transitions (random unless a sequence is loaded)\n");
	printf(" reset (i), r(i)     reset the simulator to the initial marking and re-seed (does not clear)\n");
	printf("\nSetting/Viewing State:\n");
	printf(" set <i> <expr>      execute a transition as if it were local to the i'th token\n");
	printf(" set <expr>          execute a transition as if it were remote to all tokens\n");
	printf(" force <expr>        execute a transition as if it were local to all tokens\n");
}

void real_time(prs::production_rule_set &pr, ucs::variable_set &v, vector<prs::term_index> steps = vector<prs::term_index>())
{
	prs::simulator sim(&pr, &v);

	tokenizer assignment_parser(false);
	parse_expression::composition::register_syntax(assignment_parser);

	int seed = 0;
	srand(seed);
	int enabled = 0;
	bool uptodate = false;
	int step = 0;
	int n = 0, n1 = 0, n2 = 0;
	char command[256];
	bool done = false;
	FILE *script = stdin;
	while (!done)
	{
		if (script == stdin)
		{
			printf("(prsim)");
			fflush(stdout);
		}
		if (fgets(command, 255, script) == NULL && script != stdin)
		{
			fclose(script);
			script = stdin;
			printf("(prsim)");
			fflush(stdout);
			if (fgets(command, 255, script) == NULL)
				exit(0);
		}
		int length = strlen(command);
		command[length-1] = '\0';
		length--;

		if ((strncmp(command, "help", 4) == 0 && length == 4) || (strncmp(command, "h", 1) == 0 && length == 1))
			print_command_help();
		else if ((strncmp(command, "quit", 4) == 0 && length == 4) || (strncmp(command, "q", 1) == 0 && length == 1))
			done = true;
		else if (strncmp(command, "seed", 4) == 0)
		{
			if (sscanf(command, "seed %d", &n) == 1)
			{
				seed = n;
				srand(seed);
			}
			else
				printf("error: expected seed value\n");
		}
		else if ((strncmp(command, "clear", 5) == 0 && length == 5) || (strncmp(command, "c", 1) == 0 && length == 1))
			steps.resize(step);
		else if (strncmp(command, "source", 6) == 0 && length > 7)
		{
			script = fopen(&command[7], "r");
			if (script == NULL)
			{
				printf("error: file not found '%s'", &command[7]);
				script = stdin;
			}
		}
		else if (strncmp(command, "load", 4) == 0 && length > 5)
		{
			FILE *seq = fopen(&command[5], "r");
			if (seq != NULL)
			{
				while (fgets(command, 255, seq) != NULL)
				{
					if (sscanf(command, "%d.%d", &n, &n1) == 2)
						steps.push_back(prs::term_index(n, n1));
				}
				fclose(seq);
			}
			else
				printf("error: file not found '%s'\n", &command[5]);
		}
		else if (strncmp(command, "save", 4) == 0 && length > 5)
		{
			FILE *seq = fopen(&command[5], "w");
			if (seq != NULL)
			{
				for (int i = 0; i < (int)steps.size(); i++)
					fprintf(seq, "%d.%d\n", steps[i].index, steps[i].term);
				fclose(seq);
			}
		}
		else if (strncmp(command, "reset", 5) == 0 || strncmp(command, "r", 1) == 0)
		{
			sim.reset();
			uptodate = false;
			step = 0;
			srand(seed);
		}
		else if (strncmp(command, "wait", 4) == 0 || strncmp(command, "w", 1) == 0)
			sim.encoding = sim.global;
		else if ((strncmp(command, "tokens", 6) == 0 && length == 6) || (strncmp(command, "t", 1) == 0 && length == 1))
			printf("%s\n", export_composition(sim.encoding, v).to_string().c_str());
		else if ((strncmp(command, "enabled", 7) == 0 && length == 7) || (strncmp(command, "e", 1) == 0 && length == 1))
		{
			if (!uptodate)
			{
				enabled = sim.enabled();
				uptodate = true;
			}

			for (int i = 0; i < enabled; i++)
				printf("(%d) T%d.%d:%s     ", i, sim.loaded[sim.ready[i].first].index, sim.ready[i].second, export_composition(pr.rules[sim.loaded[sim.ready[i].first].index].local_action[sim.ready[i].second], v).to_string().c_str());
			printf("\n");
		}
		else if (strncmp(command, "set", 3) == 0)
		{
			int i = 0;
			if (sscanf(command, "set %d ", &n) != 1)
			{
				n = -1;
				i = 4;
			}
			else
			{
				i = 5;
				while (i < length && command[i-1] != ' ')
					i++;
			}

			assignment_parser.insert("", string(command).substr(i));
			parse_expression::composition expr(assignment_parser);
			boolean::cube local_action = import_cube(expr, v, 0, &assignment_parser, false);
			boolean::cube remote_action = local_action.remote(v.get_groups());
			if (assignment_parser.is_clean())
			{
				sim.encoding = boolean::local_assign(sim.encoding, local_action, true);
				sim.global = boolean::local_assign(sim.global, remote_action, true);
				sim.encoding = boolean::remote_assign(sim.encoding, sim.global, true);
			}
			assignment_parser.reset();
			uptodate = false;
		}
		else if (strncmp(command, "force", 5) == 0)
		{
			if (length <= 6)
				printf("error: expected expression\n");
			else
			{
				assignment_parser.insert("", string(command).substr(6));
				parse_expression::composition expr(assignment_parser);
				boolean::cube local_action = import_cube(expr, v, 0, &assignment_parser, false);
				boolean::cube remote_action = local_action.remote(v.get_groups());
				if (assignment_parser.is_clean())
				{
					sim.encoding = boolean::local_assign(sim.encoding, remote_action, true);
					sim.global = boolean::local_assign(sim.global, remote_action, true);
				}
				assignment_parser.reset();
				uptodate = false;
			}
		}
		else if (strncmp(command, "step", 4) == 0 || strncmp(command, "s", 1) == 0)
		{
			if (sscanf(command, "step %d", &n) != 1 && sscanf(command, "s%d", &n) != 1)
				n = 1;

			for (int i = 0; i < n && (enabled != 0 || !uptodate); i++)
			{
				if (!uptodate)
				{
					enabled = sim.enabled();
					uptodate = true;
				}

				if (enabled != 0)
				{
					int firing = rand()%enabled;
					if (step < (int)steps.size())
					{
						for (firing = 0; firing < (int)sim.ready.size() &&
						(sim.loaded[sim.ready[firing].first].index != steps[step].index || sim.ready[firing].second != steps[step].term); firing++);

						if (firing == (int)sim.ready.size())
						{
							printf("error: loaded simulation does not match PRS, please clear the simulation to continue\n");
							break;
						}
					}
					else
						steps.push_back(prs::term_index(sim.loaded[sim.ready[firing].first].index, sim.ready[firing].second));

					printf("%d\tT%d.%d\t%s -> %s\n", step, sim.loaded[sim.ready[firing].first].index, sim.ready[firing].second, export_expression(sim.loaded[sim.ready[firing].first].guard_action, v).to_string().c_str(), export_composition(pr.rules[sim.loaded[sim.ready[firing].first].index].local_action[sim.ready[firing].second], v).to_string().c_str());

					//boolean::cube old = sim.encoding;
					sim.fire(firing);

					//printf("\t%s\n", export_composition(difference(old, sim.encoding), v).to_string().c_str());

					uptodate = false;
					sim.interference_errors.clear();
					sim.instability_errors.clear();
					sim.mutex_errors.clear();
					step++;
				}
			}
		}
		else if (strncmp(command, "fire", 4) == 0 || strncmp(command, "f", 1) == 0)
		{
			if (sscanf(command, "fire %d", &n) == 1 || sscanf(command, "f%d", &n) == 1)
			{
				if (!uptodate)
				{
					enabled = sim.enabled();
					uptodate = true;
				}

				if (n < enabled)
				{
					if (step < (int)steps.size())
						printf("error: deviating from loaded simulation, please clear the simulation to continue\n");
					else
					{
						steps.push_back(prs::term_index(sim.loaded[sim.ready[n].first].index, sim.ready[n].second));

						printf("%d\tT%d.%d\t%s -> %s\n", step, sim.loaded[sim.ready[n].first].index, sim.ready[n].second, export_expression(sim.loaded[sim.ready[n].first].guard_action, v).to_string().c_str(), export_composition(pr.rules[sim.loaded[sim.ready[n].first].index].local_action[sim.ready[n].second], v).to_string().c_str());

						//boolean::cube old = sim.encoding;
						sim.fire(n);

						//printf("\t%s\n", export_composition(difference(old, sim.encoding), v).to_string().c_str());

						uptodate = false;
						sim.interference_errors.clear();
						sim.instability_errors.clear();
						sim.mutex_errors.clear();
						step++;
					}
				}
				else
					printf("error: must be in the range [0,%d)\n", enabled);
			}
			else
				printf("error: expected ID in the range [0,%d)\n", enabled);
		}
		else if (length > 0)
			printf("error: unrecognized command '%s'\n", command);
	}
}

int main(int argc, char **argv)
{
	configuration config;
	config.set_working_directory(argv[0]);
	tokenizer tokens;
	parse_prs::production_rule_set::register_syntax(tokens);
	tokens.register_token<parse::block_comment>(false);
	tokens.register_token<parse::line_comment>(false);
	string sgfilename = "";
	string pnfilename = "";
	string egfilename = "";
	string gfilename = "";
	vector<prs::term_index> steps;

	bool labels = false;

	for (int i = 1; i < argc; i++)
	{
		string arg = argv[i];
		if (arg == "--help" || arg == "-h")			// Help
		{
			print_help();
			return 0;
		}
		else if (arg == "--version")	// Version Information
		{
			print_version();
			return 0;
		}
		else if (arg == "--verbose" || arg == "-v")
			set_verbose(true);
		else if (arg == "--debug" || arg == "-d")
			set_debug(true);
		else if (arg == "--labels" || arg == "-l")
			labels = true;
		else
		{
			string filename = argv[i];
			int dot = filename.find_last_of(".");
			string format = "";
			if (dot != string::npos)
				format = filename.substr(dot+1);
			if (format == "prs")
				config.load(tokens, filename, "");
			else if (format == "sim")
			{
				FILE *seq = fopen(argv[i], "r");
				char command[256];
				int n, n1, n2;
				if (seq != NULL)
				{
					while (fgets(command, 255, seq) != NULL)
					{
						if (sscanf(command, "%d.%d", &n, &n1) == 2)
							steps.push_back(prs::term_index(n, n1));
					}
					fclose(seq);
				}
				else
					printf("error: file not found '%s'\n", argv[i]);
			}
			else
				printf("unrecognized file format '%s'\n", format.c_str());
		}
	}

	if (is_clean() && tokens.segments.size() > 0)
	{
		prs::production_rule_set pr;
		ucs::variable_set v;

		tokens.increment(false);
		tokens.expect<parse_prs::production_rule_set>();
		if (tokens.decrement(__FILE__, __LINE__))
		{
			parse_prs::production_rule_set syntax(tokens);
			pr = import_production_rule_set(syntax, v, 0, &tokens, true);
		}
		pr.post_process(v);

		real_time(pr, v, steps);
	}

	complete();
	return is_clean();
}
