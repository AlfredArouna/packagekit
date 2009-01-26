// This file was generated by the Gtk# code generator.
// Any changes made will be lost if regenerated.

namespace PackageKit {

	using System;
	using System.Runtime.InteropServices;

#region Autogenerated code
	public class Status {

		[DllImport("libpackagekit-glib.dll")]
		static extern IntPtr pk_status_enum_to_text(int status);

		public static string EnumToText(PackageKit.StatusEnum status) {
			IntPtr raw_ret = pk_status_enum_to_text((int) status);
			string ret = GLib.Marshaller.Utf8PtrToString (raw_ret);
			return ret;
		}

		[DllImport("libpackagekit-glib.dll")]
		static extern int pk_status_enum_from_text(IntPtr status);

		public static PackageKit.StatusEnum EnumFromText(string status) {
			IntPtr native_status = GLib.Marshaller.StringToPtrGStrdup (status);
			int raw_ret = pk_status_enum_from_text(native_status);
			PackageKit.StatusEnum ret = (PackageKit.StatusEnum) raw_ret;
			GLib.Marshaller.Free (native_status);
			return ret;
		}

#endregion
	}
}