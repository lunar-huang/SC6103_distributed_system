package src;

import java.nio.ByteBuffer;
import java.util.Arrays;

public class Message {
    // 消息字段
    public byte messageType;      // 消息类型 (1 Byte)
    public int requestId;         // 请求ID (4 Bytes)
    public int dataLength;        // 数据长度 (4 Bytes)
    public byte[] data;           // 消息主体 (航班数据)

    // 构造函数
    public Message(byte messageType, int requestId, byte[] data) {
        this.messageType = messageType;
        this.requestId = requestId;
        this.data = data;
        this.dataLength = data.length;
    }

    // 序列化 (Marshalling) 将 Message 转换为字节数组
    public byte[] marshal() {
        ByteBuffer buffer = ByteBuffer.allocate(1 + 4 + 4 + data.length);

        buffer.put(messageType);                   // 1 byte 消息类型
        buffer.putInt(requestId);                  // 4 bytes 请求ID
        buffer.putInt(dataLength);                 // 4 bytes 数据长度
        buffer.put(data);                          // N bytes 消息主体

        return buffer.array();                     // 返回字节数组
    }

    // 反序列化 (Unmarshalling) 将字节数组转换为 Message 对象
    public static Message unmarshal(byte[] bytes) {
        ByteBuffer buffer = ByteBuffer.wrap(bytes);

        byte messageType = buffer.get();           // 读取消息类型
        int requestId = buffer.getInt();           // 读取请求ID
        int dataLength = buffer.getInt();          // 读取数据长度

        byte[] data = new byte[dataLength];        // 创建用于存储消息主体的数组
        buffer.get(data, 0, dataLength);           // 读取消息主体

        return new Message(messageType, requestId, data);
    }

    // 输出消息内容用于调试
    @Override
    public String toString() {
        return "MessageType: " + messageType +
                ", RequestId: " + requestId +
                ", DataLength: " + dataLength +
                ", Data: " + Arrays.toString(data);
    }
}
