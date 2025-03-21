// Copyright 2019 DeepMind Technologies Ltd. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "open_spiel/games/connect_four.h"

#include <algorithm>
#include <memory>
#include <utility>

namespace open_spiel {
namespace connect_four {
namespace {

// Facts about the game
const GameType kGameType{
    /*short_name=*/"connect_four",
    /*long_name=*/"Connect Four",
    GameType::Dynamics::kSequential,
    GameType::ChanceMode::kDeterministic,
    GameType::Information::kPerfectInformation,
    GameType::Utility::kZeroSum,
    GameType::RewardModel::kTerminal,
    /*max_num_players=*/2,
    /*min_num_players=*/2,
    /*provides_information_state=*/true,
    /*provides_information_state_as_normalized_vector=*/true,
    /*provides_observation=*/false,
    /*provides_observation_as_normalized_vector=*/false,
    /*parameter_specification=*/{}  // no parameters
};

std::unique_ptr<Game> Factory(const GameParameters& params) {
  return std::unique_ptr<Game>(new ConnectFourGame(params));
}

REGISTER_SPIEL_GAME(kGameType, Factory);

CellState PlayerToState(int player) {
  switch (player) {
    case 0:
      return CellState::kCross;
    case 1:
      return CellState::kNought;
    default:
      SpielFatalError(absl::StrCat("Invalid player id ", player));
  }
}

std::string StateToString(CellState state) {
  switch (state) {
    case CellState::kEmpty:
      return ".";
    case CellState::kNought:
      return "o";
    case CellState::kCross:
      return "x";
    default:
      SpielFatalError("Unknown state.");
      return "This will never return.";
  }
}
}  // namespace

CellState& ConnectFourState::CellAt(int row, int col) {
  return board_[row * kCols + col];
}

CellState ConnectFourState::CellAt(int row, int col) const {
  return board_[row * kCols + col];
}

int ConnectFourState::CurrentPlayer() const {
  if (IsTerminal()) {
    return kTerminalPlayerId;
  } else {
    return current_player_;
  }
}

void ConnectFourState::DoApplyAction(Action move) {
  SPIEL_CHECK_EQ(CellAt(kRows - 1, move), CellState::kEmpty);
  int row = 0;
  while (CellAt(row, move) != CellState::kEmpty) ++row;
  CellAt(row, move) = PlayerToState(CurrentPlayer());
  current_player_ = 1 - current_player_;
}

std::vector<Action> ConnectFourState::LegalActions() const {
  // Can move in any non-full column.
  std::vector<Action> moves;
  for (int col = 0; col < kCols; ++col) {
    if (CellAt(kRows - 1, col) == CellState::kEmpty) moves.push_back(col);
  }
  return moves;
}

std::string ConnectFourState::ActionToString(int player,
                                             Action action_id) const {
  return absl::StrCat(StateToString(PlayerToState(player)), action_id);
}

bool ConnectFourState::HasLineFrom(int player, int row, int col) const {
  return HasLineFromInDirection(player, row, col, 0, 1) ||
         HasLineFromInDirection(player, row, col, -1, -1) ||
         HasLineFromInDirection(player, row, col, -1, 0) ||
         HasLineFromInDirection(player, row, col, -1, 1);
}

bool ConnectFourState::HasLineFromInDirection(int player, int row, int col,
                                              int drow, int dcol) const {
  if (row + 4 * drow >= kRows || col + 4 * dcol >= kCols ||
      row + 4 * drow < 0 || col + 4 * dcol < 0)
    return false;
  CellState c = PlayerToState(player);
  for (int i = 0; i < 4; ++i) {
    if (CellAt(row, col) != c) return false;
    row += drow;
    col += dcol;
  }
  return true;
}

bool ConnectFourState::HasLine(int player) const {
  CellState c = PlayerToState(player);
  for (int col = 0; col < kCols; ++col) {
    for (int row = 0; row < kRows; ++row) {
      if (CellAt(row, col) == c && HasLineFrom(player, row, col)) return true;
    }
  }
  return false;
}

bool ConnectFourState::IsFull() const {
  for (int col = 0; col < kCols; ++col) {
    if (CellAt(kRows - 1, col) == CellState::kEmpty) return false;
  }
  return true;
}

ConnectFourState::ConnectFourState(int num_distinct_actions)
    : State(num_distinct_actions, kNumPlayers) {
  std::fill(begin(board_), end(board_), CellState::kEmpty);
}

std::string ConnectFourState::ToString() const {
  std::string str;
  for (int row = kRows - 1; row >= 0; --row) {
    for (int col = 0; col < kCols; ++col) {
      str.append(StateToString(CellAt(row, col)));
    }
    str.append("\n");
  }
  return str;
}
bool ConnectFourState::IsTerminal() const {
  return HasLine(0) || HasLine(1) || IsFull();
}

std::vector<double> ConnectFourState::Returns() const {
  if (HasLine(0)) return {1.0, -1.0};
  if (HasLine(1)) return {-1.0, 1.0};
  return {0.0, 0.0};
}

std::string ConnectFourState::InformationState(int player) const {
  SPIEL_CHECK_GE(player, 0);
  SPIEL_CHECK_LT(player, num_players_);
  return ToString();
}

void ConnectFourState::InformationStateAsNormalizedVector(
    int player, std::vector<double>* values) const {
  SPIEL_CHECK_GE(player, 0);
  SPIEL_CHECK_LT(player, num_players_);

  values->resize(kNumCells * kCellStates);
  std::fill(values->begin(), values->end(), 0.);
  for (int cell = 0; cell < kNumCells; ++cell) {
    (*values)[kNumCells * static_cast<int>(board_[cell]) + cell] = 1.0;
  }
}

void ConnectFourState::UndoAction(int player, Action move) {
  board_[move] = CellState::kEmpty;
  current_player_ = player;
  history_.pop_back();
}

std::unique_ptr<State> ConnectFourState::Clone() const {
  return std::unique_ptr<State>(new ConnectFourState(*this));
}

std::string ConnectFourGame::SerializeState(const State& state) const {
  return state.ToString();
}

ConnectFourGame::ConnectFourGame(const GameParameters& params)
    : Game(kGameType, params) {}

std::unique_ptr<State> ConnectFourGame::DeserializeState(
    const std::string& str) const {
  return std::unique_ptr<State>(
      new ConnectFourState(NumDistinctActions(), str));
}

ConnectFourState::ConnectFourState(int num_distinct_actions,
                                   const std::string& str)
    : State(num_distinct_actions, kNumPlayers) {
  int xs = 0;
  int os = 0;
  int r = 5;
  int c = 0;
  for (const char ch : str) {
    switch (ch) {
      case '.':
        CellAt(r, c) = CellState::kEmpty;
        break;
      case 'x':
        ++xs;
        CellAt(r, c) = CellState::kCross;
        break;
      case 'o':
        ++os;
        CellAt(r, c) = CellState::kNought;
        break;
    }
    if (ch == '.' || ch == 'x' || ch == 'o') {
      ++c;
      if (c >= kCols) {
        r--;
        c = 0;
      }
    }
  }
  SPIEL_CHECK_TRUE(xs == os || xs == (os + 1));
  SPIEL_CHECK_TRUE(r == -1 && ("Problem parsing state (incorrect rows)."));
  SPIEL_CHECK_TRUE(c == 0 &&
                   ("Problem parsing state (column value should be 0)"));
  current_player_ = (xs == os) ? 0 : 1;
}

}  // namespace connect_four
}  // namespace open_spiel
