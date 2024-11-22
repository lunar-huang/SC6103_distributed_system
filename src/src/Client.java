package src;

import java.io.IOException;
import java.net.*;

public class Client {

    private DatagramSocket socket;
    private InetAddress serverAddress;
    private int serverPort = 8080;  // 假设服务器端口是8080
    private UserInterface userInterface;

    // 构造函数，初始化与服务器的连接
    public Client() {
        try {
            socket = new DatagramSocket();
            serverAddress = InetAddress.getByName("172.20.10.10");  // 假设服务器
            startListeningToServer();
        } catch (SocketException | UnknownHostException e) {
            e.printStackTrace();
        }
    }

    // 设置 UserInterface 的引用，用于回调显示服务器的响应
    public void setUserInterface(UserInterface userInterface) {
        this.userInterface = userInterface;
    }

    // 测试与服务器的连接
    public void testConnection() {
        String request = "test_connection";  // 简单的测试连接请求
        System.out.println("testing connection to server");
        sendRequest(request);  // 发送测试请求到服务器
        System.out.println("already sent the request to server");
    }

    // 查询航班ID
    public void queryFlightId(String sourcePlace, String destinationPlace) {

        String request = "query_flight_id " + sourcePlace + " " + destinationPlace;
        sendRequest(request);  // 发送请求到服务器


//        sendRequest("query_flight_id", sourcePlace, destinationPlace);

    }

    // 查询航班信息
    public void queryFlightInfo(int flightId) {

        String request = "query_flight_info " + flightId;
        sendRequest(request);

//        sendRequest("query_flight_info", flightId);

    }

    // 预订座位
    public void makeSeatReservation(int flightId, int numSeats) {

        String request = "make_seat_reservation " + flightId + " " + numSeats;
        sendRequest(request);

//        sendRequest("make_seat_reservation", flightId, numSeats);

    }

    // 查询行李信息
    public void queryBaggageAvailability(int flightId) {

        String request = "query_baggage_availability " + flightId;
        sendRequest(request);

//        sendRequest("query_baggage_availability", flightId);

    }

    // 添加行李
    public void addBaggage(int flightId, int numBaggages) {

        String request = "add_baggage " + flightId + " " + numBaggages;
        sendRequest(request);

//        sendRequest("add_baggage", flightId, numBaggages);

    }

    // 关注航班
    public void followFlightId(int flightId) {

        String request = "follow_flight_id " + flightId;
        sendRequest(request);

//        sendRequest("follow_flight_info", flightId);

    }


    // 返回服务器地址
    public String getServerAddress() {
        System.out.println("getting server address");
        if (serverAddress != null) {
            System.out.println("Server address: " + serverAddress.getHostAddress());
            return serverAddress.getHostAddress();
        } else {
            System.out.println("Server address not found!");
            return "Server address not found!";
        }
    }


    // 发送请求到服务器
    private void sendRequest(String request) {
        try {
            byte[] requestData = request.getBytes();
            DatagramPacket packet = new DatagramPacket(requestData, requestData.length, serverAddress, serverPort);
            System.out.println("Sending request: " + request);
            socket.send(packet);

            // 启动线程接收服务器响应
//            new Thread(new ResponseListener()).start();
            System.out.println("sendRequest done");
        } catch (IOException e) {
            if (userInterface != null) {
                userInterface.displayResponse("Error sending request: " + e.getMessage());
            }
            System.out.println("Error sending request: " + e.getMessage());
        }
    }




    // 启动服务器消息监听线程
    public void startListeningToServer() {
        new Thread(new ResponseListener()).start();
    }

    // 监听服务器响应
    private class ResponseListener implements Runnable {
        @Override
        public void run() {
            try {
                // 设置socket超时时间
                socket.setSoTimeout(60000);  // 超时时间为60秒

                // 持续监听服务器的响应
                while (true) {
                    try {
                        byte[] buffer = new byte[1024];
                        DatagramPacket responsePacket = new DatagramPacket(buffer, buffer.length);
                        System.out.println("Waiting for server response...");
                        socket.receive(responsePacket);  // 接收服务器的响应

                        // 解析服务器的响应
                        String response = new String(responsePacket.getData(), 0, responsePacket.getLength());
                        System.out.println("Received response from server: " + response);

                        // 将响应结果显示在用户界面上
                        if (userInterface != null) {
                            userInterface.displayResponse("Response: " + response);
                        }

                    } catch (SocketTimeoutException e) {
                        System.out.println("Request timed out: No response from server within the timeout period.");
                        if (userInterface != null) {
                            userInterface.displayResponse("Request timed out: No response from server.");
                        }
                    }
                }
            } catch (IOException e) {
                if (userInterface != null) {
                    userInterface.displayResponse("Error receiving response: " + e.getMessage());
                }
                System.out.println("Error receiving response: " + e.getMessage());
            }
        }
    }

}
