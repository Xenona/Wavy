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

VCD::VCDData *VCDParser::getVCDData(VCDTokenStream *tokenStream) {
  this->vcd = new VCD::VCDData();
  this->tokenStream = tokenStream;
  std::stack<VCD::ScopeData> scopes;

  while (this->tokenStream->peek().type != TokenType::NIL) {
    Token token = this->tokenStream->next();
    switch (this->state) {

    case (ParserState::READING_HEADER): {

      switch (token.type) {
      case (TokenType::DateKeyword): {
        this->state = ParserState::READING_DATE;
        break;
      };

      case (TokenType::VersionKeyword): {
        this->state = ParserState::READING_VERSION;
        break;
      };

      case (TokenType::TimescaleKeyword): {
        this->state = ParserState::READING_TIMESCALE;
        break;
      }

      case (TokenType::ScopeKeyword): {
        scopes.push({.parentScopeID = ""});
        this->state = ParserState::READING_SCOPE;
        break;
      }

      case (TokenType::EnddefinitionsKeyword): {
        this->state = ParserState::READING_END_DEFINITIONS;
        break;
      }

      default: {
        this->warn("Could not handle " + token.value + " in IDLE");
      }
      };
      break;
    }

    case (ParserState::READING_DATA): {
      switch (token.type) {
      case (TokenType::SimulationTime): {
        this->vcd->timepoints.push_back(
            {.time = stoi(token.value), .data = {}});
        break;
      }

      case (TokenType::CommentKeyword): {
        this->state = ParserState::READING_COMMENT;
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
        this->vcd->timepoints.back().data.type = VCD::DumpType::ALL;
        this->state = ParserState::READING_DUMPS;
        break;
      }
      case (TokenType::DumponKeyword): {
        this->vcd->timepoints.back().data.type = VCD::DumpType::ON;
        this->state = ParserState::READING_DUMPS;
        break;
      }
      case (TokenType::DumpoffKeyword): {
        this->vcd->timepoints.back().data.type = VCD::DumpType::OFF;
        this->state = ParserState::READING_DUMPS;
        break;
      }
      case (TokenType::DumpvarsKeyword): {

        this->vcd->timepoints.back().data.type = VCD::DumpType::VARS;
        this->state = ParserState::READING_DUMPS;
        break;
      }
      default: {
        this->warn("Could not handle " + token.value + " in READING_DATA");
      }
      }
      break;
    }

    case (ParserState::READING_DATE): {
      if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::READING_HEADER;
      } else {
        this->vcd->date.date += token.value + " ";
      }
      break;
    }

    case (ParserState::READING_END_DEFINITIONS): {
      if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::READING_DATA;
      } else {
        this->warn("$enddefinitions should not contain any tokens.");
      }
      break;
    }

    case (ParserState::READING_VERSION): {
      if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::READING_HEADER;
      } else {
        this->vcd->version.version += token.value + " ";
      }
      break;
    }

    case (ParserState::READING_COMMENT): {
      if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::READING_DATA;
      } else {
        this->vcd->comments.back().comment += token.value + " ";
      }
      break;
    }

    case (ParserState::READING_TIMESCALE): {
      if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::READING_HEADER;
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
              this->error("Timescale units are incorrect");
            }
          }
        }
      }
      break;
    }

    case (ParserState::READING_SCOPE): {
      if (token.type == TokenType::Identifier) {
        if (scopes.top().type == VCD::ScopeTypes::NIL) {
          if (token.value == "begin")
            scopes.top().type = VCD::ScopeTypes::BEGIN;
          else if (token.value == "fork")
            scopes.top().type = VCD::ScopeTypes::FORK;
          else if (token.value == "function")
            scopes.top().type = VCD::ScopeTypes::FUNCTION;
          else if (token.value == "module")
            scopes.top().type = VCD::ScopeTypes::MODULE;
          else if (token.value == "task")
            scopes.top().type = VCD::ScopeTypes::TASK;
          else {
            this->warn("Scope has wrong type.");
            scopes.top().type = VCD::ScopeTypes::UNDEFINED;
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
          this->state = ParserState::READING_HEADER;
      } else if (token.type == TokenType::VarKeyword) {
        this->state = ParserState::READING_SCOPE_VAR;
        scopes.top().vars.push_back({});
      } else if (token.type == TokenType::ScopeKeyword) {
        scopes.push({.parentScopeID = scopes.top().ID});
      } else if (token.type == TokenType::UpscopeKeyword) {
        this->vcd->scopes.push_back(scopes.top());
        scopes.pop();
      }
      break;
    }

    case (ParserState::READING_SCOPE_VAR): {
      if (this->tokenStream->isInteger(token.value)) {
        scopes.top().vars.back().size = std::stoi(token.value);
      } else if (token.type == TokenType::Identifier) {
        if (scopes.top().vars.back().type == VCD::VarTypes::NO) {
          VCD::VarTypes type;
          if (token.value == "event")
            type = VCD::VarTypes::event;
          else if (token.value == "integer")
            type = VCD::VarTypes::integer;
          else if (token.value == "parameter")
            type = VCD::VarTypes::parameter;
          else if (token.value == "real")
            type = VCD::VarTypes::real;
          else if (token.value == "realtime")
            type = VCD::VarTypes::realtime;
          else if (token.value == "reg")
            type = VCD::VarTypes::reg;
          else if (token.value == "supply0")
            type = VCD::VarTypes::supply0;
          else if (token.value == "supply1")
            type = VCD::VarTypes::supply1;
          else if (token.value == "time")
            type = VCD::VarTypes::time;
          else if (token.value == "tri")
            type = VCD::VarTypes::tri;
          else if (token.value == "triand")
            type = VCD::VarTypes::triand;
          else if (token.value == "trior")
            type = VCD::VarTypes::trior;
          else if (token.value == "trireg")
            type = VCD::VarTypes::trireg;
          else if (token.value == "tri0")
            type = VCD::VarTypes::tri0;
          else if (token.value == "tri1")
            type = VCD::VarTypes::tri1;
          else if (token.value == "wand")
            type = VCD::VarTypes::wand;
          else if (token.value == "wire")
            type = VCD::VarTypes::wire;
          else if (token.value == "wor")
            type = VCD::VarTypes::wor;

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
        this->state = ParserState::READING_SCOPE;
      }
      break;
    }

      // todo
      // rename all enums
      // use back instead of size-1

    case (ParserState::READING_DUMPS): {

      if (token.type == TokenType::ScalarValueChange) {
        this->vcd->timepoints.back().data.scals.push_back(
            {.value = token.value[0], .identifier = token.value.substr(1)});
      } else if (token.type == TokenType::VectorValueChange) {
        this->vcd->timepoints.back().data.vecs.push_back(
            {.type = token.value[0], .valueVec = token.value.substr(1)});
      } else if (token.type == TokenType::Identifier) {
        this->vcd->timepoints.back().data.vecs.back().identifier = token.value;
      } else if (token.type == TokenType::EndKeyword) {
        this->state = ParserState::READING_DATA;
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
