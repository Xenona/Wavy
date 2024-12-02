#include "./vcd-parser.h"
#include "vcd-data.h"
#include "vcd-token-stream.h"
#include <QtDebug>
#include <QtLogging>
#include <cctype>
#include <cstdlib>
#include <exception>
#include <qdebug.h>
#include <stack>
#include <string>

VCDData *VCDParser::getVCDData(VCDTokenStream *tokenStream) {
  this->reset();
  VCDData *vcd = new VCDData();
  this->tokenStream = tokenStream;
  std::stack<ScopeData> scopes;
  long long prevTime = -1;

  while (this->tokenStream->peek().type != TokenType::NIL) {
    Token token;
    try {
      token = this->tokenStream->next();
    } catch (const std::exception &e) {
      this->error(std::string{"An error happened during parsing: "} + e.what(),
                  vcd);
    } catch (...) {
      qFatal() << "An unhandled exception in vcd-parser. Seems like token "
                  "stream failed.";
    }
    switch (this->state) {

    case (ParserState::Header): {

      switch (token.type) {
      case (TokenType::DateKeyword): {
        this->state = ParserState::Date;
        break;
      };

      case (TokenType::VersionKeyword): {
        this->state = ParserState::Version;
        break;
      };

      case (TokenType::TimescaleKeyword): {
        this->state = ParserState::Timescale;
        break;
      }

      case (TokenType::ScopeKeyword): {
        scopes.push({.parentScopeID = ""});
        this->state = ParserState::Scope;
        break;
      }

      case (TokenType::EnddefinitionsKeyword): {
        this->state = ParserState::EndDefinitions;
        break;
      }

      default: {
        this->warn("Could not handle " + token.value + " in IDLE", vcd);
      }
      };
      break;
    }

    case (ParserState::Data): {
      switch (token.type) {
      case (TokenType::SimulationTime): {
        long long time;
        try {
          time = std::stoll(token.value);
        } catch (const std::exception &e) {
          this->error(
              "Simulation time contains wrong characters. Cannot "
              "interpret as long long integer: " +
                  token.value +
                  ". Now assuming zero, which may lead to further errors.",
              vcd);
          time = 0;
        }
        if (prevTime >= time) {
          this->error("Simulation time should increase only, no repeating "
                      "timings, no reversed timings. Moved from time " +
                          std::to_string(prevTime) + " to time " + token.value +
                          ". Exiting.",
                      vcd);
        }
        vcd->timepoints.push_back({.time = stoi(token.value), .data = {}});
        break;
      }

      case (TokenType::CommentKeyword): {
        this->state = ParserState::Comment;
        vcd->comments.push_back({""});
        break;
      };

      case (TokenType::ScalarValueChange): {
        vcd->timepoints.back().data.scals.push_back(
            {.value = token.value[0] - '0',
             .stringValue = std::string{token.value[0]},
             .identifier = token.value.substr(1)});
        break;
      }

      case (TokenType::VectorValueChange): {
        double f = 0;
        long long ll = 0;
        if (token.value[0] == 'r' || token.value[0] == 'R') {
          f = stof(token.value.substr(1));
        }
        if (token.value[0] == 'b' || token.value[0] == 'B') {
          ll = strtoll(token.value.substr(1).c_str(), nullptr, 2);
        }
        vcd->timepoints.back().data.vecs.push_back(
            {.type = token.value[0],
             .valueVec = token.value.substr(1),
             .valueVecDec = ll,
             .valueVecDecFloat = f});
        break;
      }

      case (TokenType::Identifier): {
        vcd->timepoints.back().data.vecs.back().identifier = token.value;
        break;
      }

      case (TokenType::DumpallKeyword): {
        vcd->timepoints.back().data.type = DumpType::All;
        this->state = ParserState::Dumps;
        break;
      }
      case (TokenType::DumponKeyword): {
        vcd->timepoints.back().data.type = DumpType::On;
        this->state = ParserState::Dumps;
        break;
      }
      case (TokenType::DumpoffKeyword): {
        vcd->timepoints.back().data.type = DumpType::Off;
        this->state = ParserState::Dumps;
        break;
      }
      case (TokenType::DumpvarsKeyword): {

        vcd->timepoints.back().data.type = DumpType::Vars;
        this->state = ParserState::Dumps;
        break;
      }
      default: {
        this->warn("Could not handle " + token.value + " in READING_DATA", vcd);
      }
      }
      break;
    }

    case (ParserState::Date): {
      if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::Header;
      } else {
        vcd->date.date += token.value + " ";
      }
      break;
    }

    case (ParserState::EndDefinitions): {
      if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::Data;
      } else {
        this->warn("$enddefinitions should not contain any tokens.", vcd);
      }
      break;
    }

    case (ParserState::Version): {
      if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::Header;
      } else {
        vcd->version.version += token.value + " ";
      }
      break;
    }

    case (ParserState::Comment): {
      if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::Data;
      } else {
        vcd->comments.back().comment += token.value + " ";
      }
      break;
    }

    case (ParserState::Timescale): {
      if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::Header;
      } else {
        if (this->tokenStream->isInteger(token.value)) {
          vcd->timescale.precision = stoi(token.value);
        } else if (token.type == TokenType::Identifier) {
          if (vcd->timescale.precision) {
            vcd->timescale.unit = token.value;
          } else {
            std::string precision;
            for (char c : token.value) {
              if (this->tokenStream->isDigit(c)) {
                precision += c;
                continue;
              }
              if (this->tokenStream->isLetter(c)) {
                vcd->timescale.unit += c;
              }
            }
            vcd->timescale.precision = stoi(precision);
            if (vcd->timescale.unit != "s" && vcd->timescale.unit != "ms" &&
                vcd->timescale.unit != "us" && vcd->timescale.unit != "ns" &&
                vcd->timescale.unit != "ps") {
              this->error(
                  "Timescale units are incorrect, using default (ns) instead",
                  vcd);
              vcd->timescale.unit = "ns";
            }
          }
        }
      }
      break;
    }

    case (ParserState::Scope): {
      if (token.type == TokenType::Identifier) {
        if (scopes.top().type == ScopeTypes::NIL) {
          if (token.value == "begin")
            scopes.top().type = ScopeTypes::Begin;
          else if (token.value == "fork")
            scopes.top().type = ScopeTypes::Fork;
          else if (token.value == "function")
            scopes.top().type = ScopeTypes::Function;
          else if (token.value == "module")
            scopes.top().type = ScopeTypes::Module;
          else if (token.value == "task")
            scopes.top().type = ScopeTypes::Task;
          else {
            this->warn("Scope has wrong type.", vcd);
            scopes.top().type = ScopeTypes::Undefined;
          }
        } else if (scopes.top().name == "") {

          scopes.top().name = token.value;
          scopes.top().ID = std::to_string(scopes.size()) +
                            std::to_string(scopes.top().name.length()) +
                            std::to_string((int)token.type);
        }

        else
          this->warn(
              "Scope declaration has excess values which are to be ignored.",
              vcd);
      } else if (token.type == TokenType::EndKeyword) {
        if (scopes.empty())
          this->state = ParserState::Header;
      } else if (token.type == TokenType::VarKeyword) {
        this->state = ParserState::ScopeVar;
        scopes.top().vars.push_back({});
      } else if (token.type == TokenType::ScopeKeyword) {
        scopes.push({.parentScopeID = scopes.top().ID});
      } else if (token.type == TokenType::UpscopeKeyword) {
        vcd->scopes.push_back(scopes.top());
        scopes.pop();
      }
      break;
    }

    case (ParserState::ScopeVar): {
      if (this->tokenStream->isInteger(token.value) &&
          !scopes.top().vars.back().size) {
        scopes.top().vars.back().size = std::stoi(token.value);
      } else if (token.type == TokenType::Identifier) {
        if (scopes.top().vars.back().type == VarTypes::NIL) {
          VarTypes type;
          if (token.value == "event")
            type = VarTypes::Event;
          else if (token.value == "integer")
            type = VarTypes::Integer;
          else if (token.value == "parameter")
            type = VarTypes::Parameter;
          else if (token.value == "real")
            type = VarTypes::Real;
          else if (token.value == "realtime")
            type = VarTypes::Realtime;
          else if (token.value == "reg")
            type = VarTypes::Reg;
          else if (token.value == "supply0")
            type = VarTypes::Supply0;
          else if (token.value == "supply1")
            type = VarTypes::Supply1;
          else if (token.value == "time")
            type = VarTypes::Time;
          else if (token.value == "tri")
            type = VarTypes::Tri;
          else if (token.value == "triand")
            type = VarTypes::Triand;
          else if (token.value == "trior")
            type = VarTypes::Trior;
          else if (token.value == "trireg")
            type = VarTypes::Trireg;
          else if (token.value == "tri0")
            type = VarTypes::Tri0;
          else if (token.value == "tri1")
            type = VarTypes::Tri1;
          else if (token.value == "wand")
            type = VarTypes::Wand;
          else if (token.value == "wire")
            type = VarTypes::Wire;
          else if (token.value == "wor")
            type = VarTypes::Wor;

          scopes.top().vars.back().type = type;
        } else if (scopes.top().vars.back().identifier == "") {
          scopes.top().vars.back().identifier = token.value;
        } else if (scopes.top().vars.back().trueName == "") {
          scopes.top().vars.back().trueName = token.value;
        } else {
          this->warn(
              "Var declaration has excess tokens which are to be ignored. " +
                  token.value,
              vcd);
        }
      } else if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::Scope;
      }
      break;
    }

    case (ParserState::Dumps): {

      if (token.type == TokenType::ScalarValueChange) {
        vcd->timepoints.back().data.scals.push_back(
            {.value = token.value[0] - '0',
             .stringValue = std::string{token.value[0]},
             .identifier = token.value.substr(1)});
      } else if (token.type == TokenType::VectorValueChange) {
        double f = 0;
        long long ll = 0;
        if (token.value[0] == 'r' || token.value[0] == 'R') {
          f = stof(token.value.substr(1));
        }
        if (token.value[0] == 'b' || token.value[0] == 'B') {
          ll = strtoll(token.value.substr(1).c_str(), nullptr, 2);
        }
        vcd->timepoints.back().data.vecs.push_back(
            {.type = token.value[0],
             .valueVec = token.value.substr(1),
             .valueVecDec = ll,
             .valueVecDecFloat = f});
      } else if (token.type == TokenType::Identifier) {
        vcd->timepoints.back().data.vecs.back().identifier = token.value;
      } else if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::Data;
      }
      break;
    }
    }
  }

  return vcd;
};

VCDParser::~VCDParser() { delete this->tokenStream; }

void VCDParser::error(std::string message, VCDData *vcd) {
  vcd->errors.push_back(message);
}

void VCDParser::warn(std::string message, VCDData *vcd) {
  vcd->warns.push_back(message);
}

void VCDParser::reset() {
  if (this->tokenStream != nullptr) {
    delete this->tokenStream;
    this->tokenStream = nullptr;
  }
  this->state = ParserState::Header;
}

void VCDParser::dbg(VCDTokenStream *s) {
  std::vector<Token> tks;

  // while (s->peek().type != TokenType::NIL) {
  //   tks.push_back(s->next());
  // }

  auto data = getVCDData(s);

  int a = 5;
}
