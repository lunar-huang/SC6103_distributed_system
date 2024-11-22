package src;


import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

public class UserInterface extends JFrame {

    private Client client;  // Client 类用于与服务器交互
    private String selectedOperation;  // 记录当前选择的操作

    // GUI 组件
    private JTextArea outputArea;
    private JTextField inputField1, inputField2, inputField3;
    private JLabel inputPromptLabel1, inputPromptLabel2, inputPromptLabel3;
    private JButton executeButton, testAddressButton, quitButton, queryFlightIdButton, queryFlightInfoButton, makeReservationButton, queryBaggageButton, addBaggageButton, followFlightIdButton;

    public UserInterface(Client client) {
        this.client = client;
        this.client.setUserInterface(this);  // 设置 Client 中的回调界面

        setTitle("Flight Information System");
        setSize(600, 500);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        initGUI();
    }

    // 初始化 GUI
    private void initGUI() {
        outputArea = new JTextArea();
        outputArea.setEditable(false);

        inputField1 = new JTextField(10);
        inputField2 = new JTextField(10);
        inputField3 = new JTextField(10);

        inputPromptLabel1 = new JLabel("Input 1: ");
        inputPromptLabel2 = new JLabel("Input 2: ");
        inputPromptLabel3 = new JLabel("Input 3: ");

        executeButton = new JButton("Execute");  // 新增执行按钮
        quitButton = new JButton("Quit");

        // 操作按钮
        queryFlightIdButton = new JButton("Query Flight ID");
        queryFlightInfoButton = new JButton("Query Flight Info");
        makeReservationButton = new JButton("Make Seat Reservation");
        queryBaggageButton = new JButton("Query Baggage");
        addBaggageButton = new JButton("Add Baggage");
        followFlightIdButton = new JButton("Follow Flight Id");

        JPanel inputPanel = new JPanel();
        inputPanel.setLayout(new GridLayout(3, 2));  // 将输入框和提示标签布局为 3 行 2 列
        inputPanel.add(inputPromptLabel1);
        inputPanel.add(inputField1);
        inputPanel.add(inputPromptLabel2);
        inputPanel.add(inputField2);
        inputPanel.add(inputPromptLabel3);
        inputPanel.add(inputField3);

        JPanel buttonPanel = new JPanel();
        buttonPanel.setLayout(new GridLayout(3, 2, 10, 10));  // 使用 GridLayout 将按钮布局到中间部分
        buttonPanel.add(queryFlightIdButton);
        buttonPanel.add(queryFlightInfoButton);
        buttonPanel.add(makeReservationButton);
        buttonPanel.add(queryBaggageButton);
        buttonPanel.add(addBaggageButton);
        buttonPanel.add(quitButton);
        buttonPanel.add(followFlightIdButton);


        Container container = getContentPane();
        container.setLayout(new BorderLayout());
        container.add(new JScrollPane(outputArea), BorderLayout.CENTER);
        container.add(inputPanel, BorderLayout.NORTH);  // 输入框放在顶部
        container.add(buttonPanel, BorderLayout.SOUTH);  // 按钮放在底部
        container.add(executeButton, BorderLayout.EAST); // 执行按钮放置在右侧

        // 添加测试服务器地址按钮
        testAddressButton = new JButton("Test Server Address");
        testAddressButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                String serverAddress = client.getServerAddress();  // 获取服务器地址
                outputArea.append("Server Address: " + serverAddress + "\n");
            }
        });
        buttonPanel.add(testAddressButton);


        // 添加一个按钮用于测试连接
        JButton testConnectionButton = new JButton("Test Connection");

        testConnectionButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                String serverAddress = client.getServerAddress();  // 获取服务器地址
                // outputArea.append("Server Address: " + serverAddress + "\n");

                client.testConnection();  // 调用客户端的测试连接方法
                outputArea.append("testing connection with server: " + serverAddress + "\n");
            }
        });

        // 添加按钮到面板
        buttonPanel.add(testConnectionButton);


        // 查询航班ID按钮点击事件
        queryFlightIdButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
//                selectedOperation = "query_flight_id";  // 设置操作为查询航班ID
//                inputPromptLabel1.setText("Source Place:");
//                inputPromptLabel2.setText("Destination Place:");
//                inputPromptLabel3.setVisible(false);  // 只需要两个输入
//                inputField3.setVisible(false);
                selectedOperation = "query_flight_id";  // 设置操作为查询航班ID
                // 将所有输入框和标签设为可见
                inputPromptLabel1.setVisible(true);
                inputPromptLabel2.setVisible(true);
                inputPromptLabel3.setVisible(false);  // 只需要两个输入
                inputField1.setVisible(true);
                inputField2.setVisible(true);
                inputField3.setVisible(false);

                inputPromptLabel1.setText("Source Place:");
                inputPromptLabel2.setText("Destination Place:");
            }
        });

        // 查询航班信息按钮点击事件
        queryFlightInfoButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
//                selectedOperation = "query_flight_info";  // 设置操作为查询航班信息
//                inputPromptLabel1.setText("Flight ID:");
//                inputPromptLabel2.setVisible(false);  // 只需要一个输入
//                inputPromptLabel3.setVisible(false);
//                inputField2.setVisible(false);
//                inputField3.setVisible(false);
                selectedOperation = "query_flight_info";  // 设置操作为查询航班信息
                // 将所有输入框和标签设为可见
                inputPromptLabel1.setVisible(true);
                inputPromptLabel2.setVisible(false);  // 只需要一个输入
                inputPromptLabel3.setVisible(false);
                inputField1.setVisible(true);
                inputField2.setVisible(false);
                inputField3.setVisible(false);

                inputPromptLabel1.setText("Flight ID:");
            }
        });

        // 关注航班信息按钮点击事件
        followFlightIdButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
//                selectedOperation = "query_flight_info";  // 设置操作为查询航班信息
//                inputPromptLabel1.setText("Flight ID:");
//                inputPromptLabel2.setVisible(false);  // 只需要一个输入
//                inputPromptLabel3.setVisible(false);
//                inputField2.setVisible(false);
//                inputField3.setVisible(false);
                selectedOperation = "follow_flight_id";  // 设置操作为查询航班信息
                // 将所有输入框和标签设为可见
                inputPromptLabel1.setVisible(true);
                inputPromptLabel2.setVisible(false);  // 只需要一个输入
                inputPromptLabel3.setVisible(false);
                inputField1.setVisible(true);
                inputField2.setVisible(false);
                inputField3.setVisible(false);

                inputPromptLabel1.setText("Flight ID:");
            }
        });



        // 预订座位按钮点击事件
        makeReservationButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
//                selectedOperation = "make_seat_reservation";  // 设置操作为预订座位
//                inputPromptLabel1.setText("Flight ID:");
//                inputPromptLabel2.setText("Number of Seats:");
//                inputPromptLabel3.setVisible(false);
//                inputField3.setVisible(false);
                selectedOperation = "make_seat_reservation";  // 设置操作为预订座位
                // 将所有输入框和标签设为可见
                inputPromptLabel1.setVisible(true);
                inputPromptLabel2.setVisible(true);
                inputPromptLabel3.setVisible(false);  // 只需要两个输入
                inputField1.setVisible(true);
                inputField2.setVisible(true);
                inputField3.setVisible(false);

                inputPromptLabel1.setText("Flight ID:");
                inputPromptLabel2.setText("Number of Seats:");
            }
        });

        // 查询行李按钮点击事件
        queryBaggageButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
//                selectedOperation = "query_baggage_availability";  // 设置操作为查询行李
//                inputPromptLabel1.setText("Flight ID:");
//                inputPromptLabel2.setVisible(false);  // 只需要一个输入
//                inputPromptLabel3.setVisible(false);
//                inputField2.setVisible(false);
//                inputField3.setVisible(false);
                selectedOperation = "query_baggage_availability";  // 设置操作为查询行李
                // 将所有输入框和标签设为可见
                inputPromptLabel1.setVisible(true);
                inputPromptLabel2.setVisible(false);  // 只需要一个输入
                inputPromptLabel3.setVisible(false);
                inputField1.setVisible(true);
                inputField2.setVisible(false);
                inputField3.setVisible(false);

                inputPromptLabel1.setText("Flight ID:");
            }
        });

        // 添加行李按钮点击事件
        addBaggageButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
//                selectedOperation = "add_baggage";  // 设置操作为添加行李
//                inputPromptLabel1.setText("Flight ID:");
//                inputPromptLabel2.setText("Number of Baggages:");
//                inputPromptLabel3.setVisible(false);
//                inputField3.setVisible(false);
                selectedOperation = "add_baggage";  // 设置操作为添加行李
                // 将所有输入框和标签设为可见
                inputPromptLabel1.setVisible(true);
                inputPromptLabel2.setVisible(true);
                inputPromptLabel3.setVisible(false);  // 只需要两个输入
                inputField1.setVisible(true);
                inputField2.setVisible(true);
                inputField3.setVisible(false);

                inputPromptLabel1.setText("Flight ID:");
                inputPromptLabel2.setText("Number of Baggages:");
            }
        });

        // 执行按钮点击事件，根据选择的操作执行不同的客户端方法
        executeButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                try {
                    switch (selectedOperation) {
                        case "query_flight_id":
                            String sourcePlace = inputField1.getText();
                            String destinationPlace = inputField2.getText();
                            client.queryFlightId(sourcePlace, destinationPlace);
                            break;
                        case "query_flight_info":
                            int flightId = Integer.parseInt(inputField1.getText());
                            client.queryFlightInfo(flightId);
                            break;
                        case "follow_flight_id":
                            flightId = Integer.parseInt(inputField1.getText());
                            client.followFlightId(flightId);
                            break;
                        case "make_seat_reservation":
                            flightId = Integer.parseInt(inputField1.getText());
                            int numSeats = Integer.parseInt(inputField2.getText());
                            client.makeSeatReservation(flightId, numSeats);
                            break;
                        case "query_baggage_availability":
                            flightId = Integer.parseInt(inputField1.getText());
                            client.queryBaggageAvailability(flightId);
                            break;
                        case "add_baggage":
                            flightId = Integer.parseInt(inputField1.getText());
                            int numBaggages = Integer.parseInt(inputField2.getText());
                            client.addBaggage(flightId, numBaggages);
                            break;
                        default:
                            outputArea.append("No operation selected or invalid input.\n");
                    }
                } catch (NumberFormatException ex) {
                    outputArea.append("Invalid input: " + ex.getMessage() + "\n");
                }
            }
        });

        // 退出按钮点击事件
        quitButton.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                System.exit(0);  // 退出程序
            }
        });
    }

    // 显示服务器的响应
    public void displayResponse(String response) {
        outputArea.append(response + "\n");
    }

    public static void main(String[] args) {
        // 创建 Client 实例（用于与服务器通信）
        Client client = new Client();

        // 创建并显示 UserInterface
        SwingUtilities.invokeLater(() -> {
            UserInterface ui = new UserInterface(client);
            ui.setVisible(true);
        });
    }
}
