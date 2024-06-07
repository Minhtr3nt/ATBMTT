#include <iostream>
#include <random>
#include <cmath>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QCryptographicHash>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QString>
#include <QByteArray>
#include <QDebug>
#include <cstdint>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
// Hàm kiểm tra xem một số có phải là số nguyên tố hay không
bool isPrime(int number) {
    if (number <= 1) {
        return false;
    }
    for (int i = 2; i <= sqrt(number); ++i) {
        if (number % i == 0) {
            return false;
        }
    }
    return true;
}
// Hàm tìm một số nguyên tố trong khoảng [min, max]
int findPrime(int min ,int max){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(min, max);

    int number = dist(gen);
    while(!isPrime(number)){
        number  = dist(gen);
    }
    return number;
}
//Hàm kiểm tra tính hợp lệ của alpha có là số nguyên thủy thuộc Zp*
bool isPrimitiveElement(int alpha, int p){
    int phi= p-1;// Số Euler của p
    for(int i=2; i<phi; ++i){
        if(static_cast<int>(pow(alpha, i))%p==1){
            return false;
        }
    }
    return true;
}
//Hàm chọn phần tử nguyên thủy alpha trong nhóm Zp*
int choosePrimitiveElement(int p){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(2, p - 1);
    int alpha = dist(gen);
    while(!isPrimitiveElement(alpha, p)){
        alpha = dist(gen);
    }
    return alpha;
}
int choosePrivateKey(int p){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(2, p-1);
    int a = dist(gen);
    return a;
}
// Hàm kiểm tra tính hợp lệ của k
int gcd(int k, int p) {
    if (p == 0) return k;
    else return gcd(p, k % p);
}

int choosePrivateKeySign(int p){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(2, p-1);
    int k = dist(gen);
    while(gcd(k, p-1)!=1){
        k = dist(gen);
    }
    return k;
}
// Hàm tính \( k^x \mod n \)
long long modPow(long long base, long long exp, long long mod) {
    long long result = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = (result * base) % mod;
        }
        exp = exp >> 1;
        base = (base * base) % mod;
    }
    return result;
}
void MainWindow::on_pushButton_10_clicked()
{
    int p = findPrime(100, 1000); // Chọn số nguyên tố p
    int alpha = choosePrimitiveElement(p); // Chọn phần tử nguyên thủy alpha trong nhóm Zp*
    int a= choosePrivateKey(p);
    int beta = modPow(alpha, a, p);
    if(beta<0){
        beta = beta+ p;
    }
    int k = choosePrivateKeySign(p);
    int gamma = modPow(alpha, k,p);
    if(gamma<0){
        gamma = gamma+ p;
    }
    ui->textEdit_3->setText(QString::number(p));
    ui->textEdit_5->setText(QString::number(alpha));
    ui->textEdit_4->setText(QString::number(beta));
    ui->textEdit_6->setText(QString::number(a));
    ui->textEdit_7->setText(QString::number(k));
    ui->textEdit_8->setText(QString::number(gamma));
}


void MainWindow::on_pushButton_clicked()
{
    // Mở hộp thoại chọn file
    QString filePath = QFileDialog::getOpenFileName(nullptr, "Open File", QDir::homePath(), "Text Files (*.txt)");

    if (!filePath.isEmpty()) {
        // Đọc nội dung file và hiển thị lên QTextEdit
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content = in.readAll();
            ui->textEdit->setPlainText(content);
            file.close();
        } else {
            qDebug() << "Failed to open file:" << filePath;
        }
    }
}
QString hashString(const QString& message){
    QByteArray messageBytes  = message.toUtf8();
    QByteArray hashedBytes = QCryptographicHash::hash(messageBytes, QCryptographicHash::Sha256);
    return hashedBytes.toHex();
}

// Hàm tính nghịch đảo modulo (modular multiplicative inverse) của a modulo m
long long modInverse(long long a, long long m) {
    a = (a % m + m) % m; // Đảm bảo a không âm
    long long m0 = m, t, q;
    long long x0 = 0, x1 = 1;
    if (m == 1)
        return 0;
    while (a > 1) {
        q = a / m;
        t = m;
        m = a % m, a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }
    if (x1 < 0)
        x1 += m0;
    return x1;
}
// Hàm tính toán omega
long long calculateOmega(long long decVal, long long a, long long gamma, long long k, long long p) {
    // Tính nghịch đảo modulo của k
    long long kInverse = modInverse(k, p - 1);

    // Tính a và gamma modulo p
    a = (a % p + p) % p;
    gamma = (gamma % p + p) % p;

    // Tính toán omega an toàn chỉ một lần
    long long omega = ((decVal - (a * gamma) % (p - 1) + (p - 1)) * kInverse) % (p - 1);
    if (omega < 0) {
        omega += (p - 1);
    }
    return omega;
}
void MainWindow::on_pushButton_4_clicked()  // Hàm tạo chữ ký
{
    // Lấy giá trị đầu vào từ giao diện người dùng
    QString pText = ui->textEdit_3->toPlainText();
    QString betaText = ui->textEdit_4->toPlainText();
    QString alphaText = ui->textEdit_5->toPlainText();
    QString aText = ui->textEdit_6->toPlainText();
    QString kText = ui->textEdit_7->toPlainText();
    QString gammaText = ui->textEdit_8->toPlainText();
    QString messageText = ui->textEdit->toPlainText();

    // Chuyển đổi các giá trị từ chuỗi sang số nguyên
    bool conversionSuccess;
    long long p = pText.toLongLong(&conversionSuccess);
    long long beta = betaText.toLongLong(&conversionSuccess);
    long long alpha = alphaText.toLongLong(&conversionSuccess);
    long long a = aText.toLongLong(&conversionSuccess);
    long long k = kText.toLongLong(&conversionSuccess);
    long long gamma = gammaText.toLongLong(&conversionSuccess);

    // Chuyển đổi thông điệp sang dạng mảng byte
    QByteArray message = messageText.toLatin1();

    // Băm thông điệp
    QByteArray hash = QCryptographicHash::hash(message, QCryptographicHash::Sha256);
    long long decVal = hash.toLongLong(nullptr, 16);
    ui->textEdit_12->setPlainText(hash.toHex());
    // Tính toán omega an toàn
    long long omega = calculateOmega(decVal, a, gamma, k, p);

    // Hiển thị kết quả
    ui->textEdit_9->setText(QString::number(omega));
}







void MainWindow::on_pushButton_2_clicked() //Chuyển văn bản và chữ ký sang kiểm tra
{
    QString message= ui->textEdit->toPlainText();
    ui->textEdit_2->setText(message);
    QString message1= ui->textEdit_12->toPlainText();
    ui->textEdit_13->setText(message1);
    QString Sign = ui->textEdit_9->toPlainText();
    ui->textEdit_10->setPlainText(Sign);

}


long long power(long long base, long long exponent) {  //hàm lũy thừa
    long long result = 1;
    while (exponent > 0) {
        result = result*base;
        exponent--;
    }
    return result;
}
void MainWindow::on_pushButton_5_clicked()
{
    // Mở hộp thoại chọn file
    QString filePath = QFileDialog::getOpenFileName(nullptr, "Open File", QDir::homePath(), "Text Files (*.txt)");

    if (!filePath.isEmpty()) {
        // Đọc nội dung file và hiển thị lên QTextEdit
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content = in.readAll();
            ui->textEdit_2->setPlainText(content);
            file.close();
        } else {
            qDebug() << "Failed to open file:" << filePath;
        }
    }
}


void MainWindow::on_pushButton_7_clicked()
{

    // Mở hộp thoại chọn file
    QString filePath = QFileDialog::getOpenFileName(nullptr, "Open File", QDir::homePath(), "Text Files (*.txt)");

    if (!filePath.isEmpty()) {
        // Đọc nội dung file và hiển thị lên QTextEdit
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content = in.readAll();
            ui->textEdit_10->setPlainText(content);
            file.close();
        } else {
            qDebug() << "Failed to open file:" << filePath;
        }
    }
}
void MainWindow::on_pushButton_3_clicked()// Đọc file
{
    QString filePath = "D:\\ky6\\ATBMTT\\untitled\\filevanban.txt";
    QFile file(filePath);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        // Tạo đối tượng QTextStream và ghi chuỗi vào file
        QString messageText = ui->textEdit->toPlainText();
        QTextStream out(&file);
        out << messageText ;

        // Đóng file
        file.close();
    } else {

        qDebug() << "Không thể mở file để ghi.";
    }
    QString filePath_1 ="D:\\ky6\\ATBMTT\\untitled\\Chuky.txt";
    QFile file_1(filePath_1);

    if (file_1.open(QIODevice::WriteOnly | QIODevice::Text)) {
        // Tạo đối tượng QTextStream và ghi chuỗi vào file
        QString Sign = ui->textEdit_9->toPlainText();
        QTextStream out(&file_1);
        out << Sign ;

        // Đóng file
        file_1.close();
    } else {

        qDebug() << "Không thể mở file để ghi.";
    }
}

    void MainWindow::on_pushButton_8_clicked() // Kiểm tra chữ ký xem có đúng không
    {
        QString pText = ui->textEdit_3->toPlainText();
        QString betaText = ui->textEdit_4->toPlainText();
        QString alphaText = ui->textEdit_5->toPlainText();
        QString aText = ui->textEdit_6->toPlainText();
        QString kText = ui->textEdit_7->toPlainText();
        QString gammaText = ui->textEdit_8->toPlainText();
       // QString messageText = ui->textEdit_2->toPlainText();
        QString SignText = ui->textEdit_10->toPlainText();

        bool conversionSuccess;
        long long p = pText.toLongLong(&conversionSuccess);
        long long beta = betaText.toLongLong(&conversionSuccess);
        long long alpha = alphaText.toLongLong(&conversionSuccess);
        long long a = aText.toLongLong(&conversionSuccess);
        long long k = kText.toLongLong(&conversionSuccess);
        long long gamma = gammaText.toLongLong(&conversionSuccess);
        long long omega = SignText.toLongLong(&conversionSuccess);

        // Chuyển đổi thông điệp sang dạng mảng byte
        QString hash = ui->textEdit_13->toPlainText();

        // Cập nhật giá trị decVal với bản băm mới

        long long decVal = hash.toLongLong(nullptr, 16);

        // Tính (beta^gamma * gamma^omega) % p và (alpha^decVal) % p
        long long lhs = (modPow(beta, gamma, p) * modPow(gamma, omega, p)) % p;
        long long rhs = modPow(alpha, decVal, p);
        QString hash_1 = ui->textEdit_12->toPlainText();
        QString hash_2 = ui->textEdit_13->toPlainText();
        if(hash_1 != hash_2){
            if (lhs != rhs) {
                ui->textEdit_11->setPlainText("Chữ ký không hợp lệ\n do sửa đổi văn bản\n và sửa đổi chữ ký");
                return;
            }
            ui->textEdit_11->setPlainText("Chữ ký không hợp lệ\n do sửa đổi văn bản");

            return;
        }
        if (lhs == rhs) {
            ui->textEdit_11->setPlainText("Chữ ký hợp lệ");
            return;
        }else{
            if(hash_1 != hash_2){
                ui->textEdit_11->setPlainText("Chữ ký không hợp lệ\n do sửa đổi văn bản\n và sửa đổi chữ ký");
                return;
            }
            ui->textEdit_11->setPlainText("Chữ ký không hợp lệ\n do sửa đổi hàm ký");

            return;
        }


    }




    void MainWindow::on_textEdit_2_textChanged()
    {
        QString messageText = ui->textEdit_2->toPlainText();
         QByteArray message = messageText.toLatin1();
        QByteArray hash = QCryptographicHash::hash(message, QCryptographicHash::Sha256);
        ui->textEdit_13->setPlainText(hash.toHex());
    }




