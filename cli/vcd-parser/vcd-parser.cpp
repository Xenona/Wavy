#include "./vcd-parser.h"
#include "vcd-data.h"
#include "vcd-token-stream.h"
#include <QtDebug>
#include <QtLogging>
#include <cctype>
#include <qdebug.h>
#include <stack>
#include <string>

int test(int a) { return a; }

VCDData *VCDParser::getVCDData(VCDTokenStream *tokenStream) {
  this->vcd = new VCDData();
  this->tokenStream = tokenStream;
  std::stack<ScopeData> scopes;

  while (this->tokenStream->peek().type != TokenType::NIL) {
    Token token = this->tokenStream->next();
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
        this->warn("Could not handle " + token.value + " in IDLE");
      }
      };
      break;
    }

    case (ParserState::Data): {
      switch (token.type) {
      case (TokenType::SimulationTime): {
        this->vcd->timepoints.push_back(
            {.time = stoi(token.value), .data = {}});
        break;
      }

      case (TokenType::CommentKeyword): {
        this->state = ParserState::Comment;
        this->vcd->comments.push_back({""});
        break;
      };

      case (TokenType::ScalarValueChange): {
        this->vcd->timepoints.back().data.scals.push_back(
            {.value = token.value[0], .identifier = token.value.substr(1)});
        break;
      }

      case (TokenType::VectorValueChange): {
        this->vcd->timepoints.back().data.vecs.push_back(
            {.type = token.value[0], .valueVec = token.value.substr(1)});
        break;
      }

      case (TokenType::Identifier): {
        this->vcd->timepoints.back().data.vecs.back().identifier = token.value;
        break;
      }

      case (TokenType::DumpallKeyword): {
        this->vcd->timepoints.back().data.type = DumpType::All;
        this->state = ParserState::Dumps;
        break;
      }
      case (TokenType::DumponKeyword): {
        this->vcd->timepoints.back().data.type = DumpType::On;
        this->state = ParserState::Dumps;
        break;
      }
      case (TokenType::DumpoffKeyword): {
        this->vcd->timepoints.back().data.type = DumpType::Off;
        this->state = ParserState::Dumps;
        break;
      }
      case (TokenType::DumpvarsKeyword): {

        this->vcd->timepoints.back().data.type = DumpType::Vars;
        this->state = ParserState::Dumps;
        break;
      }
      default: {
        this->warn("Could not handle " + token.value + " in READING_DATA");
      }
      }
      break;
    }

    case (ParserState::Date): {
      if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::Header;
      } else {
        this->vcd->date.date += token.value + " ";
      }
      break;
    }

    case (ParserState::EndDefinitions): {
      if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::Data;
      } else {
        this->warn("$enddefinitions should not contain any tokens.");
      }
      break;
    }

    case (ParserState::Version): {
      if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::Header;
      } else {
        this->vcd->version.version += token.value + " ";
      }
      break;
    }

    case (ParserState::Comment): {
      if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::Data;
      } else {
        this->vcd->comments.back().comment += token.value + " ";
      }
      break;
    }

    case (ParserState::Timescale): {
      if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::Header;
      } else {
        if (this->tokenStream->isInteger(token.value)) {
          this->vcd->timescale.precision = stoi(token.value);
        } else if (token.type == TokenType::Identifier) {
          if (this->vcd->timescale.precision) {
            this->vcd->timescale.unit = token.value;
          } else {
            std::string precision;
            for (char c : token.value) {
              if (this->tokenStream->isDigit(c)) {
                precision += c;
                continue;
              }
              if (this->tokenStream->isLetter(c)) {
                this->vcd->timescale.unit += c;
              }
            }
            this->vcd->timescale.precision = stoi(precision);
            if (this->vcd->timescale.unit != "s" &&
                this->vcd->timescale.unit != "ms" &&
                this->vcd->timescale.unit != "us" &&
                this->vcd->timescale.unit != "ns" &&
                this->vcd->timescale.unit != "ps") {
              this->error("Timescale units are incorrect, using default (ns) instead");
              this->vcd->timescale.unit = "ns";
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
            this->warn("Scope has wrong type.");
            scopes.top().type = ScopeTypes::Undefined;
          }
        } else if (scopes.top().name == "") {

          scopes.top().name = token.value;
          scopes.top().ID = std::to_string(scopes.size()) +
                            std::to_string(scopes.top().name.length());
        }

        else
          this->warn(
              "Scope declaration has excess values which are to be ignored.");
      } else if (token.type == TokenType::EndKeyword) {
        if (scopes.empty())
          this->state = ParserState::Header;
      } else if (token.type == TokenType::VarKeyword) {
        this->state = ParserState::ScopeVar;
        scopes.top().vars.push_back({});
      } else if (token.type == TokenType::ScopeKeyword) {
        scopes.push({.parentScopeID = scopes.top().ID});
      } else if (token.type == TokenType::UpscopeKeyword) {
        this->vcd->scopes.push_back(scopes.top());
        scopes.pop();
      }
      break;
    }

    case (ParserState::ScopeVar): {
      if (this->tokenStream->isInteger(token.value)) {
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
              "Var declaration has excess tokens which are to be ignored.");
        }
      } else if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::Scope;
      }
      break;
    }

    case (ParserState::Dumps): {

      if (token.type == TokenType::ScalarValueChange) {
        this->vcd->timepoints.back().data.scals.push_back(
            {.value = token.value[0], .identifier = token.value.substr(1)});
      } else if (token.type == TokenType::VectorValueChange) {
        this->vcd->timepoints.back().data.vecs.push_back(
            {.type = token.value[0], .valueVec = token.value.substr(1)});
      } else if (token.type == TokenType::Identifier) {
        this->vcd->timepoints.back().data.vecs.back().identifier = token.value;
      } else if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::Data;
      }
      break;
    }
    }
  }

  return this->vcd;
};

void VCDParser::error(std::string message) {
  this->vcd->errors.push_back(message);
}

void VCDParser::warn(std::string message) {
  this->vcd->warns.push_back(message);
}

void VCDParser::dbg(VCDTokenStream *s) {
  std::vector<Token> tks;

  // while (s->peek().type != TokenType::NIL) {
  //   tks.push_back(s->next());
  // }

  auto data = getVCDData(s);

  int a = 5;
}
