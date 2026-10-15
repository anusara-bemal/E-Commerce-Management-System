@echo off
title E-Commerce Management System

echo ========================================
echo   E-Commerce Management System
echo ========================================
echo.

gcc src\main.c -o ecommerce.exe

if errorlevel 1 (
echo.
echo Compilation failed! Please check your code.
pause
exit /b 1
)

echo Compilation successful!
echo.
ecommerce.exe

echo.
pause
