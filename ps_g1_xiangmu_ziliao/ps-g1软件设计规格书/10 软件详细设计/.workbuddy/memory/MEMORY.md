# MEMORY.md - PS-G1项目长期记忆

## 项目概况
- PS-G1 EV充电控制系统软件开发
- 软件分层：APP层（ASW）、RTE层、BSW层、驱动层
- ASW模块：CP、Charging、Diag、Discharging、LED、Factory、Log、Calibration、电压测量、电流测量
- BSW模块：AdcF、AdcM、EcuM、WdgM

## 已完成工作
- 2026-04-24：基于14个详细设计文档，整理出270个单元测试用例，输出文件：PS-G1单元测试用例清单.md
  - 覆盖ASW 10个模块 + BSW 4个模块
  - 包含外部接口函数56个 + 内部函数95个
  - 用例设计覆盖：正常路径、边界值、异常路径、状态机、持续条件、恢复、稳定性7类

## 技术备注
- .doc文件提取：使用olefile库解析Word二进制格式（COM自动化对部分文件会超时）
- 用户要求用三线表格形式列举用例
