//
//  check_epson_printer.h
//  check_espon_printer
//
//  Created by Steve Baker on 1/2/12.
//  Copyright 2012 XECKO LLC. All rights reserved.
//

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <exception>
#include <iostream>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <vector>

namespace po=boost::program_options;
namespace re=boost::xpressive;

// Program Options
struct Options {
  std::string host;                   // The host name to contact
  std::string community;              // The community name
  int sequence;                       // Variable sequence number
  int warn_level;                     // Values below this level are warnings
  int critical_level;                 // Values below this level are critical
};

using boost::format;
using std::cerr;
using std::cout;
using std::endl;
using std::exception;
using std::string;
using std::vector;

// Function declarations
Options GetOptions(int argc, char * const argv[]);

// SNMP Levels
const std::string SNMP_LEVEL_OK   = "OK";
const std::string SNMP_LEVEL_WARN = "WARNING";
const std::string SNMP_LEVEL_CRIT = "CRITICAL";

// Service Values
const int SVC_COMPONENT        = 6;
const int SVC_MAX_CAPACITY     = 8;
const int SVC_CURRENT_CAPACITY = 9;

// Return Status
const int RETURN_OK            = 0;
const int RETURN_WARN          = 1;
const int RETURN_CRIT          = 2;
const int RETURN_UNKNOWN       = 3;

