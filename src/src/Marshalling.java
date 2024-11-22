package src;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;

public class Marshalling {

    // 序列化 Message 对象
    public static byte[] marshalMessage(Message message) {
        ByteBuffer buffer = ByteBuffer.allocate(1 + 4 + 4 + message.data.length);

        buffer.put(message.messageType);  // 1 byte 消息类型
        buffer.putInt(message.requestId); // 4 bytes 请求ID
        buffer.putInt(message.dataLength);// 4 bytes 数据长度
        buffer.put(message.data);         // N bytes 数据主体

        return buffer.array();
    }

    // 反序列化为 Message 对象
    public static Message unmarshalMessage(byte[] bytes) {
        ByteBuffer buffer = ByteBuffer.wrap(bytes);

        byte messageType = buffer.get();  // 读取消息类型
        int requestId = buffer.getInt();  // 读取请求ID
        int dataLength = buffer.getInt(); // 读取数据长度
        byte[] data = new byte[dataLength]; // 读取数据内容
        buffer.get(data);

        return new Message(messageType, requestId, data);
    }

    // 序列化 Flight 对象
    public static byte[] marshalFlight(Flight flight) {
        ByteBuffer buffer = ByteBuffer.allocate(4 + 4 + flight.sourcePlace.length() + 4 + flight.destinationPlace.length() + 4 + 4);

        buffer.putInt(flight.flightId);                                // 4 bytes 航班ID
        buffer.putInt(flight.sourcePlace.length());                    // 4 bytes 出发地长度
        buffer.put(flight.sourcePlace.getBytes(StandardCharsets.UTF_8));// N bytes 出发地
        buffer.putInt(flight.destinationPlace.length());               // 4 bytes 目的地长度
        buffer.put(flight.destinationPlace.getBytes(StandardCharsets.UTF_8)); // N bytes 目的地
        buffer.putFloat(flight.airfare);                               // 4 bytes 票价
        buffer.putInt(flight.seatAvailability);                        // 4 bytes 座位可用数量

        return buffer.array();
    }

    // 反序列化为 Flight 对象
    public static Flight unmarshalFlight(byte[] bytes) {
        ByteBuffer buffer = ByteBuffer.wrap(bytes);

        int flightId = buffer.getInt();                             // 读取航班ID
        int sourceLength = buffer.getInt();                         // 读取出发地长度
        byte[] sourceBytes = new byte[sourceLength];
        buffer.get(sourceBytes);
        String sourcePlace = new String(sourceBytes, StandardCharsets.UTF_8); // 读取出发地

        int destinationLength = buffer.getInt();                    // 读取目的地长度
        byte[] destinationBytes = new byte[destinationLength];
        buffer.get(destinationBytes);
        String destinationPlace = new String(destinationBytes, StandardCharsets.UTF_8); // 读取目的地

        float airfare = buffer.getFloat();                          // 读取票价
        int seatAvailability = buffer.getInt();                     // 读取座位可用数量

        return new Flight(flightId, sourcePlace, destinationPlace, airfare, seatAvailability);
    }
}
