//
//  main.cpp
//  check_espon_printer
//
//  Created by Steve Baker on 1/2/12.
//  Copyright 2012 XECKO LLC. All rights reserved.
//

#include "check_epson_printer.h"

int main (int argc, char * const argv[])
{
  int return_value = RETURN_OK;
  
  Options opt = GetOptions(argc, argv);

  // Initialize snmp library
  init_snmp("check_epson_printer");

  // Initialize snmp session
  struct snmp_session session;
  snmp_sess_init(&session);
  session.peername      = const_cast<char *>(opt.host.c_str());
  session.version       = SNMP_VERSION_1;
  session.community_len = opt.community.length();
  session.community     = 
    reinterpret_cast<unsigned char *>(const_cast<char *>(opt.community.c_str()));

  // Establish the session
  struct snmp_session *ss = snmp_open(&session);
  if(!ss) {
    snmp_perror("Unable to open snmp session");
    snmp_log(LOG_ERR, "Unable to open snmp session\n");
    exit(RETURN_UNKNOWN);
  }

  // Create the PDU for our request
  vector<string> oids;
  oids.push_back(str(format("SNMPv2-SMI::mib-2.43.11.1.1.%2%.1.%1%") % 
                     opt.sequence % SVC_COMPONENT));
  oids.push_back(str(format("SNMPv2-SMI::mib-2.43.11.1.1.%2%.1.%1%") % 
                     opt.sequence % SVC_MAX_CAPACITY));
  oids.push_back(str(format("SNMPv2-SMI::mib-2.43.11.1.1.%2%.1.%1%") % 
                     opt.sequence % SVC_CURRENT_CAPACITY));

  struct snmp_pdu *pdu = snmp_pdu_create(SNMP_MSG_GET);
  for (vector<string>::iterator it=oids.begin(); it!=oids.end(); it++) {
    oid anOID[MAX_OID_LEN];
    size_t anOID_len = MAX_OID_LEN;
    read_objid((*it).c_str(), anOID, &anOID_len);
    snmp_add_null_var(pdu, anOID, anOID_len);
  }

  // Send the request
  struct snmp_pdu *response;
  int status = snmp_synch_response(ss, pdu, &response);

  // Process the response
  if(status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
    int position = 1;
    string component;
    long max_value;
    long current_value;
    for(struct variable_list *vars = response->variables; 
        vars; vars = vars->next_variable, ++position) {
      switch (position) {
      case 1: { // First variable is the name of the cartridge
        string value(reinterpret_cast<char *>(vars->val.string));
        //component = value;
        re::sregex rex = re::sregex::compile("\\s+");
        component = regex_replace(value, rex, "_");
        break;
      }

      case 2: { // Second variable is the max capacity of the cartridge
        max_value = *(vars->val.integer);
        break;
      }

      case 3: { // Third variable is the remaining capacity of the cartridge
        current_value = *(vars->val.integer);
        break;
      }

      default:
        break;
      }
    }
    
    // Display the response
    int percent_remaining = int((static_cast<double>(current_value) /
                                 static_cast<double>(max_value)) * 100.0);
    string label = str(format("%1%") % percent_remaining);
    string level = SNMP_LEVEL_OK;
    if(percent_remaining <= opt.warn_level) {
      level = SNMP_LEVEL_WARN;
      label = str(format("*%1%*") % percent_remaining);
      return_value = RETURN_WARN;
    }
    if(percent_remaining <= opt.critical_level) {
      level = SNMP_LEVEL_CRIT;
      label = str(format("*%1%*") % percent_remaining);
      return_value = RETURN_CRIT;
    }
    cout << format("CARTRIDGE %1% - %3%: %2%%% | component=%3% percent_remaining=%4%\n") % 
      level % label % component % percent_remaining;
    
    // Display the error if there was one
  } else {
    if (status == STAT_SUCCESS) {
      cerr << format("Error in packet\nReason: %1%\n") % 
        snmp_errstring(static_cast<int>(response->errstat));
    } else {
      snmp_sess_perror("snmpget", ss);
    }
  }

  // Free resources
  if (response) {
    snmp_free_pdu(response);
  }
  snmp_close(ss);

  return return_value;
}

//------------------------------------------------------------------------------
// Purpose    : Parse the command line and config file
// Parameters : argc, argv
// Returns    : Filled-in Options structure
//------------------------------------------------------------------------------
Options GetOptions(int argc, char * const argv[])
{

  // Read command line options
  Options opt;
  po::options_description desc("Allowed options");
  desc.add_options()
  ("help,h", "Show this message")
  ("host,H", po::value<string>(&opt.host)->default_value("localhost"),
   "Host to contact")
  ("community,C", po::value<string>(&opt.community)->default_value("public"),
   "Community string")
  ("sequence", po::value<int>(&opt.sequence)->default_value(1),
   "Variable Sequence Number")
  ("warning,w", po::value<int>(&opt.warn_level)->default_value(10),
   "Warning Level")
  ("critical,c", po::value<int>(&opt.critical_level)->default_value(2),
   "Critical Level")
  ;

  // Load configuration
  po::variables_map vm;
  try {
    po::store(po::parse_command_line(argc, argv, desc), vm);
  } catch(exception& e) {
    cerr << "Unable to parse command line: " << e.what() << endl;
    exit(RETURN_UNKNOWN);
  }
  po::notify(vm);
  if (vm.count("help")) {
    cout << desc << "\n";
    exit(RETURN_UNKNOWN);
  }

  return opt;
}

