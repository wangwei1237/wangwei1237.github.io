# 加载必要的包
library(BradleyTerry2)
library(dplyr)

# 读取数据
matches <- read.csv("matches_2.csv", stringsAsFactors = FALSE)

# 查看数据概况
cat("数据概况:\n")
print(head(matches))
cat("\n总比赛场数:", nrow(matches), "\n\n")

# 统计每个选手的胜负情况
cat("各选手胜负统计:\n")
wins <- table(matches$winner)
losses <- table(matches$loser)
all_players <- unique(c(matches$winner, matches$loser))
stats <- data.frame(
  Player = all_players,
  Wins = as.numeric(wins[all_players]),
  Losses = as.numeric(losses[all_players])
)
stats$Total <- stats$Wins + stats$Losses
stats$WinRate <- round(stats$Wins / stats$Total, 3)
print(stats)

cat("\n", rep("=", 60), "\n\n", sep = "")

# 拟合Bradley-Terry模型
# 方法1: 手动创建配对比较数据框
# 统计每对选手之间的胜负次数
player_pairs <- matches |>
  group_by(winner, loser) |>
  summarise(wins = n(), .groups = "drop")

# 创建完整的配对数据(包括0胜的情况)
all_players <- sort(unique(c(matches$winner, matches$loser)))
all_combinations <- expand.grid(player1 = all_players,
                                player2 = all_players,
                                stringsAsFactors = FALSE)
all_combinations <- all_combinations[all_combinations$player1 != all_combinations$player2, ]

# 合并胜场数据
bt_data <- all_combinations %>%
  left_join(player_pairs, by = c("player1" = "winner", "player2" = "loser")) %>%
  rename(win1 = wins) %>%
  left_join(player_pairs, by = c("player1" = "loser", "player2" = "winner")) %>%
  rename(win2 = wins) %>%
  mutate(win1 = ifelse(is.na(win1), 0, win1),
         win2 = ifelse(is.na(win2), 0, win2))

# 只保留有比赛的配对
bt_data <- bt_data %>%
  filter(win1 + win2 > 0)

print("转换后的数据格式:")
print(bt_data)

cat("\n", rep("=", 60), "\n\n", sep="")

# 关键步骤：将player1和player2转换为因子，并设置相同的水平
bt_data$player1 <- factor(bt_data$player1, levels = all_players)
bt_data$player2 <- factor(bt_data$player2, levels = all_players)

# 拟合模型（使用默认参考类别）
model <- BTm(cbind(win1, win2), player1, player2, data = bt_data)

# 显示模型摘要
cat("Bradley-Terry模型结果:\n")
print(summary(model))

cat("\n", rep("=", 60), "\n\n", sep = "")

# 提取能力参数(对数几率)
abilities <- BTabilities(model)
print("选手能力参数(log-odds):")
print(abilities)

cat("\n", rep("=", 60), "\n\n", sep = "")

# 计算95%置信区间
conf_intervals <- confint(model, level = 0.95)
print("能力参数的95%置信区间(log-odds):")
print(conf_intervals)

cat("\n", rep("=", 60), "\n\n", sep = "")

# 将能力参数转换为概率尺度(相对于平均水平)
# 计算相对强度(exponential of abilities)
relative_strength <- exp(coef(model))
cat("相对强度(相对于参考选手A):\n")
print(relative_strength)

cat("\n", rep("=", 60), "\n\n", sep = "")

# ========== 重新参数化：所有选手都有置信区间 ==========
cat("重新参数化模型 - 所有选手都有置信区间:\n\n")

# 计算所有选手的对数能力参数（包括A）
# A的系数为0（参考类别）
abilities_all <- c(A = 0, coef(model))

# 计算方差-协方差矩阵
vcov_matrix <- vcov(model)

# 为A添加行和列（方差为0，协方差为0）
n_players <- length(abilities_all)
vcov_all <- matrix(0, n_players, n_players)
rownames(vcov_all) <- colnames(vcov_all) <- names(abilities_all)
vcov_all[2:n_players, 2:n_players] <- vcov_matrix

# 方法1: 计算相对于平均水平的能力参数
mean_ability <- mean(abilities_all)
abilities_centered <- abilities_all - mean_ability

# 使用Delta方法计算标准误
# 对于centered abilities: SE(ability_i - mean) 
se_centered <- rep(0, n_players)
names(se_centered) <- names(abilities_all)

for (i in 1:n_players) {
  # 构造线性组合系数: c_i = 1, c_j = -1/n for all j
  contrast <- rep(-1/n_players, n_players)
  contrast[i] <- contrast[i] + 1
  
  # SE = sqrt(c' * Vcov * c)
  se_centered[i] <- sqrt(t(contrast) %*% vcov_all %*% contrast)
}

# 计算95%置信区间
z_value <- qnorm(0.975)  # 95% CI
ci_lower_centered <- abilities_centered - z_value * se_centered
ci_upper_centered <- abilities_centered + z_value * se_centered

cat("相对于平均水平的能力参数:\n")
result_centered <- data.frame(
  Player = names(abilities_centered),
  Ability = round(abilities_centered, 4),
  SE = round(se_centered, 4),
  CI_Lower = round(ci_lower_centered, 4),
  CI_Upper = round(ci_upper_centered, 4)
)
print(result_centered)

cat("\n", rep("=", 60), "\n\n", sep="")

# 转换为Elo评分（所有选手都相对于平均1500分）
base_elo <- 1500
elo_scores_centered <- base_elo + 400 * abilities_centered / log(10)
elo_ci_lower <- base_elo + 400 * ci_lower_centered / log(10)
elo_ci_upper <- base_elo + 400 * ci_upper_centered / log(10)

results_final <- data.frame(
  Player = names(elo_scores_centered),
  Elo_Rating = round(elo_scores_centered, 1),
  SE = round(400 * se_centered / log(10), 1),
  CI_Lower = round(elo_ci_lower, 1),
  CI_Upper = round(elo_ci_upper, 1),
  CI_Width = round(elo_ci_upper - elo_ci_lower, 1)
)
results_final <- results_final[order(-results_final$Elo_Rating), ]

cat("最终结果 - Elo评分及95%置信区间（所有选手）:\n")
print(results_final, row.names = FALSE)

cat("\n注: 所有Elo评分都相对于平均水平1500分\n")

cat("\n", rep("=", 60), "\n\n", sep="")