//
//  main.cpp
//  TicTacToe
//
//  Created by Chidi Imala on 11/13/24.
//

#include <iostream>
#include <vector>
#include <string>
#include <typeinfo>
#include <optional>
#include <random>
#include <memory>

struct BoardPosition{
    BoardPosition(int row_,int col_) : row(row_), col(col_){}
    int row;
    int col;
};

enum class GameWinner{
    kTie = 0,
    kPlayer1,
    kPlayer2
};

enum class GameState{
    kGameOngoing = 0,
    kGameOver
};


enum class BoardState{
    kBlank = 0,
    kX = 1,
    kO = 2
};

struct GameStatus{
    GameStatus() : state(GameState::kGameOngoing),winner(std::nullopt){}
    GameStatus(GameState state_, std::optional<GameWinner> winner_) : state(state_), winner(winner_) {}
    GameState state;
    std::optional<GameWinner> winner;
};

std::ostream& operator<<(std::ostream& out,BoardState board_state){
    switch(board_state){
        case BoardState::kO:
            out << "O";
            break;
        case BoardState::kX:
            out << "X";
            break;
        case BoardState::kBlank:
            out << "Blank";
            break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out,BoardPosition spot){
    out << " (" << spot.row << ", " << spot.col << ") ";
    return out;
}

class Player{
public:
    Player() = default;
    explicit Player(std::string name_) : name(std::move(name_)){};
    virtual std::optional<BoardPosition> Move(const std::vector<std::vector<BoardState>>& board) = 0;
    virtual ~Player() = default;
    bool operator==(const Player& rhs) const{
        return name == rhs.name && typeid(*this) == typeid(rhs);
    }
    const std::string& GetName() const{
        return name;
    }
    std::vector<BoardPosition> GetPossibleMoves(const std::vector<std::vector<BoardState>>& board){
        std::vector<BoardPosition> possible_moves;
        for(int row = 0; row < board.size(); ++row){
            for (int col = 0; col < board[0].size(); col++){
                if(board[row][col] == BoardState::kBlank)
                    possible_moves.push_back(BoardPosition(row,col));
            }
        }
        return possible_moves;
    }
protected:
    std::string name;
    
};

class HumanPlayer : public Player{
public:
    HumanPlayer() : Player("HumanPlayer"){}
    std::optional<BoardPosition> Move(const std::vector<std::vector<BoardState>>& board) override{
        int move_index = -1;
        auto moves_list = GetPossibleMoves(board);
        // this should not be possible
        if(moves_list.empty()){
            return std::nullopt;
        }
        
        for(int index = 0; index < moves_list.size(); index++){
            std::cout << "index: " << index << " -> (" << moves_list[index].row << "," << moves_list[index].col << ") " << std::endl;
        }
        std::cout << "Enter move choice (0-indexed) between 0 -> " << moves_list.size() - 1 << std::endl;
        while(true){
            std::cin >> move_index;
            if(std::cin.fail()){
                std::cin.clear(); // Clear error flag
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
                std::cout << "Invalid input. Please enter a number between 0 and " << moves_list.size() - 1 << std::endl;
                continue;
            }
            if(move_index >=0 && move_index < moves_list.size()){
                break;
            }
        }

        
        return moves_list[move_index];
        
    }
};

class RandomPlayer : public Player{
    using Player::Player;
public:
    RandomPlayer() : Player("Random Player") {};
    std::optional<BoardPosition> Move(const std::vector<std::vector<BoardState>>& board) override{
        auto moves_list = GetPossibleMoves(board);
        if(moves_list.empty()){
            return std::nullopt;
        }
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dis(0, moves_list.size() - 1);
        size_t random_index = dis(gen);
        return moves_list[random_index];
        
    }
};

class TicTacToe {
public:
    // Create a public interface to create and size a new board.
    TicTacToe() = delete;
    ~TicTacToe() = default;
    TicTacToe(int N,std::shared_ptr<Player> player_1_,std::shared_ptr<Player> player_2_) : width(N),height(N),game_state(N,std::vector<BoardState>(N,BoardState::kBlank)),move_count(0),row_sums(width,0),col_sums(height,0),diag_sums(2){
        player_1 = player_1_;
        player_2 = player_2_;
        active_player = this->player_1;
        inactive_player = this->player_2;
    }
    
    const GameStatus ApplyMove(BoardPosition move){
        // set board state
        // return result
        auto result = SetBoardState(move,active_player);
        if(result){
            move_count++;
            if(CheckWin(move)){
                std::cout << "We have a winner " << std::endl;
                auto winner = active_player == player_1 ? GameWinner::kPlayer1 : GameWinner::kPlayer2;
                return GameStatus{GameState::kGameOver,winner};
            }
        }
        else{
            return GameStatus{GameState::kGameOver,GameWinner::kTie};
        }
        
        std::swap(active_player,inactive_player);
        if (move_count == width * height) {
            return GameStatus{GameState::kGameOver, GameWinner::kTie};
        }
        return GameStatus{GameState::kGameOngoing,std::nullopt};
    }
    
    void PlayTicTacToe(){
        GameStatus game_result;
        while(game_result.state == GameState::kGameOngoing){
            
            auto curr_move = active_player->Move(GetBoardState());
            if(!curr_move.has_value()){
                game_result.state = GameState::kGameOver;
                break;
            }
            if(!IsSpotLegal(curr_move.value())){
                std::cout << "An illegal move was made by " << active_player->GetName() << std::endl;
                std::cout << "Try again" << std::endl;
                continue;
            }
            game_result =  ApplyMove(curr_move.value());
        }
        std::cout << "Game Over! ";
        if (game_result.state == GameState::kGameOver) {
            if(game_result.winner.has_value()){
                if (game_result.winner == GameWinner::kPlayer1) {
                    std::cout << player_1->GetName() << "wins !" << std::endl;
                } else if (game_result.winner == GameWinner::kPlayer2) {
                    std::cout << player_2->GetName() << "wins !" << std::endl;
                    
                }
            }
            else{
                std::cout << "It's a tie" << std::endl;
            }
        }
    }
    
    bool IsSpotLegal(BoardPosition spot){
        return IsSpotOpen(spot) && IsSpotinBoard(spot);
    }
    
    bool IsSpotOpen(BoardPosition spot){
        return game_state[spot.row][spot.col] == BoardState::kBlank;
    }
    
    bool IsSpotX(BoardPosition spot){
        return game_state[spot.row][spot.col] == BoardState::kX;
    }
    
    bool IsSpotO(BoardPosition spot){
        return game_state[spot.row][spot.col] == BoardState::kO;
    }
    
    bool IsSpotinBoard(BoardPosition spot){
        return (spot.row >= 0 && spot.row < width) && (spot.col >= 0 && spot.col < height);
    }
    
    bool CheckWin(BoardPosition start_check){
        // check all the rows
        return CheckRowFaster() || CheckColFaster() || CheckDiagonalFaster();
    }
    bool CheckDiagonalFaster(){
        for(auto value: diag_sums){
            if(value == width || value == -width){
                return true;
            }
        }
        return false;
    }
    bool CheckDiagonal(BoardPosition start_check){
        // iterate from current position to top right position
        bool bottom_right_to_top_left_diagonal = true;
        int bottom_right_to_top_left_diagonal_count = 1;
        bool bottom_left_to_top_right_diagonal = true;
        int bottom_left_to_top_right_diagonal_count = 1;
    
        for(int row = start_check.row - 1, col = start_check.col + 1;row >=0 && col < game_state[0].size(); row--,col++){
            bottom_left_to_top_right_diagonal_count++;
            if(game_state[start_check.row][start_check.col] != game_state[row][col]){
                bottom_left_to_top_right_diagonal = false;
                break;
            }
        }
        for(int row = start_check.row + 1, col = start_check.col - 1;row < game_state.size() && col >=0; row++,col--){
            bottom_left_to_top_right_diagonal_count++;
            if(game_state[start_check.row][start_check.col] != game_state[row][col]){
                bottom_left_to_top_right_diagonal = false;
                break;
            }
        }
        
        for(int row = start_check.row - 1,col = start_check.col - 1;row >=0 && col >= 0; row--,col--){
            bottom_right_to_top_left_diagonal_count++;
            if(game_state[start_check.row][start_check.col] != game_state[row][col]){
                bottom_right_to_top_left_diagonal = false;
                break;
            }
        }
        
        for(int row = start_check.row + 1, col = start_check.col + 1;row < game_state.size() && col < game_state[0].size(); row++,col++){
            bottom_right_to_top_left_diagonal_count++;
            if(game_state[start_check.row][start_check.col] != game_state[row][col]){
                bottom_right_to_top_left_diagonal = false;
                break;
            }
        }
        // iterate from current position to bottom left position
        return (bottom_right_to_top_left_diagonal && bottom_right_to_top_left_diagonal_count == width) || (bottom_left_to_top_right_diagonal && bottom_left_to_top_right_diagonal_count == width );
    }
    
    bool CheckRowFaster(){
        for(auto value: row_sums){
            if(value == width || value == -width){
                return true;
            }
        }
        return false;
    }
    
    bool CheckRow(BoardPosition start_check){
        auto expected_symbol = game_state[start_check.row][start_check.col];
        for(auto col : game_state[start_check.row]){
            if(col != expected_symbol){
                return false;
            }
        }
        return true;
    }
    
    bool CheckColFaster(){
        for(auto value: col_sums){
            if(value == height || value == -height){
                return true;
            }
        }
        return false;
    }
    
    bool CheckCol(BoardPosition start_check){
        auto expected_symbol = game_state[start_check.row][start_check.col];
        for(int row = 0; row < game_state.size(); row++){
            if(game_state[row][start_check.col] != expected_symbol){
                return false;
            }
        }
        return true;
    }
                               
   const std::vector<std::vector<BoardState>>& GetBoardState(){
       
       return game_state;
   }
    
    bool SetBoardState(BoardPosition spot, const std::shared_ptr<Player>& player){
        if(!IsSpotLegal(spot))
            return false;
        if(player == player_1){
            std::cout << player->GetName() << " Just played X at " << spot <<std::endl;
            game_state[spot.row][spot.col] = BoardState::kX;
            row_sums[spot.row] += 1;
            col_sums[spot.col] += 1;
            if(spot.row == spot.col){
                diag_sums[0] += 1;
            }
            // row = 0, col = 2, 3 - 2 - 1
            // row = 1, col = 1, 3 - 1 - 1
            // row = 2, col = 0, 3 - 0 - 1
            else if(spot.row == width - spot.col - 1 ){
                diag_sums[1] +=1;
            }
        }
        else{
            std::cout << player->GetName() << " Just played O at " << spot << std::endl;
            game_state[spot.row][spot.col] = BoardState::kO;
            row_sums[spot.row] -= 1;
            col_sums[spot.col] -= 1;
            if(spot.row == spot.col){
                diag_sums[0] -= 1;
            }
            // row = 0, col = 2, 3 - 2 - 1
            // row = 1, col = 1, 3 - 1 - 1
            // row = 2, col = 0, 3 - 0 - 1
            else if(spot.row == width - spot.col - 1 ){
                diag_sums[1] -=1;
            }
        }
        
        return true;
    }
    
  // Create a method that provides an API to make a new move.
  // This should also provide the result of the move (invalid, win, continue, etc.)
  // Performance should scale linearly (or better) with board size.
  
  private:
  // storage for the game state.
    std::vector<std::vector<BoardState>> game_state; //! Board representation
    size_t move_count; //! Moves
    int width; //! Width
    int height;
    std::vector<int> row_sums;
    std::vector<int> col_sums;
    std::vector<int> diag_sums;
    std::shared_ptr<Player> player_1;
    std::shared_ptr<Player> player_2;
    std::shared_ptr<Player> active_player;
    std::shared_ptr<Player> inactive_player;
};


int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    RandomPlayer random_player_1;
    HumanPlayer human_player_1;
    TicTacToe tic_tac_player (3,std::make_shared<RandomPlayer>(random_player_1),std::make_shared<HumanPlayer>(human_player_1));
    
    tic_tac_player.PlayTicTacToe();
    return 0;
}
