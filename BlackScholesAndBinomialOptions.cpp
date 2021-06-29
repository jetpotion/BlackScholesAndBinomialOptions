// BlackScholesAndBinomialOptions.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <numeric>
#include <fstream>
#include <random>
#include <sstream>
/**
 * @brief 
 * Some assumptions about the binomial
*/

class Option
{

    enum Type_Options
    {
        EURO,
        AMERICAN,
    };
    enum Type_of_options
    {
        CALL,
        PUT,
    };
    
    double M_Time_to_expiration;
    std::vector<double>M_history_of_stockprice;
    double M_Strike_Price;
    double M_Risk_Free_IR;
    double M_volatility;
    double  M_dividend;
   
    std::size_t M_time_steps;
    Type_of_options M_type;
   Type_Options M_euro_or_american;
    
    double optionprice;
public:
    Option()
    {

    }


     void LoadPrices(std::string path)
    {
        std::ifstream f(path, std::ios::in | std::ios::out);
        std::string s{};
        f.ignore(1000, 1);
        //skip the first line 
        std::getline(f, s);
        std::stringstream newline(s);
        std::string news;
        while (std::getline(newline, news, ','))
        {
            (*this).M_history_of_stockprice.push_back(std::stod(news));
        }
        
    }
    void Computevolatility()
    {
       assert(M_history_of_stockprice.size() == 0 , "Cannot just divide zero");
       double mean = std::accumulate((*this).M_history_of_stockprice.begin(),(*this). M_history_of_stockprice.end(), 0.0) /(*this).M_history_of_stockprice.size();
       (*this).M_volatility = std::sqrt(std::accumulate((*this).M_history_of_stockprice.begin(), (*this).M_history_of_stockprice.end(), 0.0, [=](double& a ,double& b)
              {
               return  a + std::pow(b - mean, 2);
           }) / (*this). M_history_of_stockprice.size() - 1);
    }
    //Compute the biomial option price using 
    void ComputeBinoPrice()
    {
        double delta_T = (*this).M_Time_to_expiration / (*this).M_time_steps;
        double  up = std::exp((*this).M_volatility * std::sqrt((*this).M_Time_to_expiration));
        double down = 1 / up;
        double prob = std::exp((*this).M_Risk_Free_IR * delta_T) - down / (up - down);
        double inv_prob = 1 - prob;
        std::vector<std::vector<double>>dp((*this).M_time_steps + 1, std::vector<double>((*this).M_time_steps + 1));
        //using the dynamic programming we can comput  option price using a bottom up appaorahc 
        dp[0][0] = M_history_of_stockprice.back();
        for (int x = 1; x < dp.size(); x++)
        {
            dp[x][0] = dp[x - 1][0] * up;
            for (int j = 1; j < dp[0].size(); j++)
            {
                dp[x][j] = dp[x - 1][j - 1] * down;
            }
        }
        for (int x = 0; x < dp.size(); x++)
            if (M_type ==  PUT)
                dp[M_time_steps][x] = std::max(M_Strike_Price - dp[M_time_steps][x],0.0);
            else if(M_type == CALL)
                dp[M_time_steps][x] = std::max(dp[M_time_steps][x]  - M_Strike_Price , 0.0);

        for (int m = 0; m < M_time_steps; m++)
        {
            int i = M_time_steps - m - 1;
            for (int j = 0; j < i + 1; j++)
            {
                dp[i][j] = prob * dp[i + 1][j] + inv_prob * dp[i + 1][j + 1]
                    * std::exp(-M_Risk_Free_IR * delta_T);
                if (M_type == PUT)
                    dp[i][j] = std::max(dp[i][j], M_Strike_Price - dp[i][j]);
                else if (M_type == CALL)
                    dp[i][j] = std::max(dp[i][j], dp[i][j]  - M_Strike_Price);
                
            }
        }

        optionprice = dp[0][0];

    }
    //Function that calculates the black scholes for continous time distribution 
    std::pair<double, double>  CalculateBlackScholes()
    {
        std::normal_distribution<double>norm(0, 1);
        double d1 = std::log(M_history_of_stockprice.back() / M_Strike_Price +
            (M_Risk_Free_IR + std::pow(M_volatility, 2) / 2) * M_Time_to_expiration / M_volatility * std::sqrt(M_Time_to_expiration));
        double d2 = d1 - M_volatility * std::sqrt(M_Time_to_expiration);
        double call = M_history_of_stockprice.back() * std::exp(-M_dividend * M_Time_to_expiration) * norm(d1) - M_Strike_Price * std::exp(-M_Risk_Free_IR * M_Time_to_expiration) * norm(d2);
        d2 *=  -1;
        d1 *= -1;
        double put = M_Strike_Price * std::exp(-M_Risk_Free_IR * M_Time_to_expiration) * norm(d2) - M_history_of_stockprice.back() * std::exp(-M_dividend * M_Time_to_expiration) * norm(d1);
        return std::make_pair(call, put);
    }


};
int main()
{
    std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
